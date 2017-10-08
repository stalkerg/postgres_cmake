macro(replace_python2 input_file output_file)
	file(READ ${input_file} tmp_string)
	string(REGEX REPLACE
		"except \([a-zA-Z\\.]*\), *\([a-zA-Z]*\):"
		"except \\1 as \\2:"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"<type 'exceptions\\.\([a-zA-Z]*\)'>" "<class '\\1'>"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"<type 'long'>" "<class 'int'>"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"\([0-9][0-9]*\)L" "\\1"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"\([ [{]\)u\"" "\\1\""
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"\([ [{]\)u'" "\\1'"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"def next" "def __next__"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"LANGUAGE plpythonu" "LANGUAGE plpython3u"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"LANGUAGE plpython2u" "LANGUAGE plpython3u"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"EXTENSION \([^ ]*_\)+plpythonu" "EXTENSION \\1plpython3u"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"EXTENSION plpythonu" "EXTENSION plpython3u"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"EXTENSION \([^ ]*_\)+plpython2u" "EXTENSION \\1plpython3u"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"EXTENSION plpython2u" "EXTENSION plpython3u"
		tmp_string
		"${tmp_string}")
	string(REGEX REPLACE
		"installing required extension \"plpython2u\""
		"installing required extension \"plpython3u\""
		tmp_string
		"${tmp_string}")
	file(WRITE ${output_file} "${tmp_string}")
endmacro()

macro(replace_python_files r_regress_files)
	file(MAKE_DIRECTORY sql/python3 expected/python3 results/python3)
	set(adition_clean "")
	set(regress_files3 "")
	foreach(rfile ${r_regress_files})
		replace_python2("sql/${rfile}.sql" "sql/python3/${rfile}.sql")
		if(rfile STREQUAL "plpython_types")
			replace_python2("expected/plpython_types_3.out" "expected/python3/${rfile}.out")
		else()
			replace_python2("expected/${rfile}.out" "expected/python3/${rfile}.out")
		endif()
		set(adition_clean "${adition_clean};sql/python3/${rfile}.sql;expected/python3/${rfile}.out;")
		set(regress_files3 "${regress_files3};python3/${rfile}")
	endforeach(rfile)
	SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${adition_clean}")
endmacro()
