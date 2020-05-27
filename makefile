webserver: main.cpp ./timer/timer.h  ./3rdparty/json/json-forwards.h ./3rdparty/json/json.h ./3rdparty/json/jsoncpp.cpp ./threadpool/threadpool.h ./http/http_conn.cpp ./http/http_conn.h ./lock/locker.h ./CGImysql/sql_connection_pool.cpp ./CGImysql/sql_connection_pool.h
		g++ -g -o webserver main.cpp ./timer/timer.h  ./3rdparty/json/json-forwards.h ./3rdparty/json/json.h ./3rdparty/json/jsoncpp.cpp ./threadpool/threadpool.h ./http/http_conn.cpp ./http/http_conn.h ./lock/locker.h ./CGImysql/sql_connection_pool.cpp ./CGImysql/sql_connection_pool.h -lpthread -lmysqlclient
clean:
		rm -r webserver
		