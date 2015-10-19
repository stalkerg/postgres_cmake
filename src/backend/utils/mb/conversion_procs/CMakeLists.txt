set(procs_list 
	euc_kr_and_mic
	euc_tw_and_big5
	latin2_and_win1250
	latin_and_mic
	utf8_and_ascii
	utf8_and_big5
	utf8_and_cyrillic
	utf8_and_euc_cn
	utf8_and_euc_jp
	utf8_and_euc_kr
	utf8_and_euc_tw
	utf8_and_gb18030
	utf8_and_gbk
	utf8_and_iso8859
	utf8_and_iso8859_1
	utf8_and_johab
	utf8_and_sjis
	utf8_and_win
	utf8_and_uhc
	utf8_and_euc2004
	utf8_and_sjis2004
	euc2004_sjis2004
)

foreach(loop_var IN ITEMS ${procs_list})
	message(STATUS "procs ${loop_var}")
	add_library(${loop_var} SHARED ${loop_var}/${loop_var}.c)
	install(TARGETS ${loop_var}
			RUNTIME DESTINATION ${PGBINDIR}
			LIBRARY DESTINATION ${LIBDIR})#
endforeach(loop_var)

install(FILES conversion_create.sql
		DESTINATION ${PGSHAREDIR})