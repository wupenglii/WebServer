test: sign.cpp ../lock/locker.h sql_connection_pool.cpp sql_connection_pool.h
		g++ -g -o test sign.cpp ../lock/locker.h sql_connection_pool.cpp sql_connection_pool.h  -lpthread -lmysqlclient
clean:
		rm -r test
		