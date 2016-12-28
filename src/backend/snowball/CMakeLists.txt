include_directories(
	"${PROJECT_SOURCE_DIR}/src/include/snowball"
	"${PROJECT_SOURCE_DIR}/src/include/snowball/libstemmer"
)

set(snow_languages
	"danish\;danish"
	"dutch\;dutch"
	"english\;english"
	"finnish\;finnish"
	"french\;french"
	"german\;german"
	"hungarian\;hungarian"
	"italian\;italian"
	"norwegian\;norwegian"
	"portuguese\;portuguese"
	"romanian\;romanian"
	"russian\;english"
	"spanish\;spanish"
	"swedish\;swedish"
	"turkish\;turkish"
)

#Create snowball_create.sql
file(STRINGS snowball.sql.in snowball_sql_tmpl NEWLINE_CONSUME)
file(STRINGS snowball_func.sql.in snowball_func_sql_tmpl NEWLINE_CONSUME)

file(WRITE "snowball_create.sql" ${snowball_func_sql_tmpl})

foreach(pair_language ${snow_languages})
	LIST(GET pair_language 0 lang)
	LIST(GET pair_language 1 ascii_lang)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stopwords/${lang}.stop")
		set(stopwords ", StopWords=${lang}")
	else()
		set(stopwords "")
	endif()
	string(REPLACE "_LANGNAME_"
		"${lang}" out_string
		"${snowball_sql_tmpl}")
	string(REPLACE "_DICTNAME_"
		"${lang}_stem" out_string
		"${out_string}")
	string(REPLACE "_CFGNAME_"
		"${lang}" out_string
		"${out_string}")
	string(REPLACE "_ASCDICTNAME_"
		"${ascii_lang}_stem" out_string
		"${out_string}")
	string(REPLACE "_NONASCDICTNAME_"
		"${lang}_stem" out_string
		"${out_string}")
	string(REPLACE "_STOPWORDS_"
		"${stopwords}" out_string
		"${out_string}")
	file(APPEND "snowball_create.sql" ${out_string})
endforeach()


set(snowball_SRCS
	dict_snowball.c
	libstemmer/api.c
	libstemmer/utilities.c
	libstemmer/stem_ISO_8859_1_danish.c
	libstemmer/stem_ISO_8859_1_dutch.c
	libstemmer/stem_ISO_8859_1_english.c
	libstemmer/stem_ISO_8859_1_finnish.c
	libstemmer/stem_ISO_8859_1_french.c
	libstemmer/stem_ISO_8859_1_german.c
	libstemmer/stem_ISO_8859_1_hungarian.c
	libstemmer/stem_ISO_8859_1_italian.c
	libstemmer/stem_ISO_8859_1_norwegian.c
	libstemmer/stem_ISO_8859_1_porter.c
	libstemmer/stem_ISO_8859_1_portuguese.c
	libstemmer/stem_ISO_8859_1_spanish.c
	libstemmer/stem_ISO_8859_1_swedish.c
	libstemmer/stem_ISO_8859_2_romanian.c
	libstemmer/stem_KOI8_R_russian.c
	libstemmer/stem_UTF_8_danish.c
	libstemmer/stem_UTF_8_dutch.c
	libstemmer/stem_UTF_8_english.c
	libstemmer/stem_UTF_8_finnish.c
	libstemmer/stem_UTF_8_french.c
	libstemmer/stem_UTF_8_german.c
	libstemmer/stem_UTF_8_hungarian.c
	libstemmer/stem_UTF_8_italian.c
	libstemmer/stem_UTF_8_norwegian.c
	libstemmer/stem_UTF_8_porter.c
	libstemmer/stem_UTF_8_portuguese.c
	libstemmer/stem_UTF_8_romanian.c
	libstemmer/stem_UTF_8_russian.c
	libstemmer/stem_UTF_8_spanish.c
	libstemmer/stem_UTF_8_swedish.c
	libstemmer/stem_UTF_8_turkish.c
)

add_library(dict_snowball ${PLUGIN_TYPE} ${snowball_SRCS})
target_link_libraries(dict_snowball postgres)
if (MSVC)
	gen_def(dict_snowball)
	target_link_libraries(dict_snowball pgcommon pgport)
endif()
set_target_properties(dict_snowball PROPERTIES PREFIX "")
install(TARGETS dict_snowball
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

install(FILES snowball_create.sql
		DESTINATION ${PGSHAREDIR})

install(FILES
		stopwords/danish.stop
		stopwords/dutch.stop
		stopwords/english.stop
		stopwords/finnish.stop
		stopwords/french.stop
		stopwords/german.stop
		stopwords/hungarian.stop
		stopwords/italian.stop
		stopwords/norwegian.stop
		stopwords/portuguese.stop
		stopwords/russian.stop
		stopwords/spanish.stop
		stopwords/swedish.stop
		stopwords/turkish.stop
		DESTINATION ${PGSHAREDIR}/tsearch_data)
