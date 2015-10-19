#TODO compile timezones

install(FILES 
			tznames/Africa.txt
			tznames/America.txt
			tznames/Antarctica.txt
			tznames/Asia.txt
			tznames/Atlantic.txt
			tznames/Australia.txt
			tznames/Etc.txt
			tznames/Europe.txt
			tznames/Indian.txt
			tznames/Pacific.txt
			tznames/Default
			tznames/Australia
			tznames/India
		DESTINATION ${PGSHAREDIR}/timezonesets)