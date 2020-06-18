webserver: main.cpp ./timer/timer.h  ./3rdparty/json/json-forwards.h ./3rdparty/json/json.h ./3rdparty/json/jsoncpp.cpp ./threadpool/threadpool.h ./http/http_conn.cpp ./http/http_conn.h ./lock/locker.h ./log/log.cpp ./log/log.h ./log/block_queue.h ./CGImysql/sql_connection_pool.cpp ./CGImysql/sql_connection_pool.h
		g++ -g -o webserver main.cpp ./timer/timer.h  ./3rdparty/json/json-forwards.h ./3rdparty/json/json.h ./3rdparty/json/jsoncpp.cpp ./threadpool/threadpool.h ./http/http_conn.cpp ./http/http_conn.h ./lock/locker.h ./log/log.cpp ./log/log.h ./log/block_queue.h ./CGImysql/sql_connection_pool.cpp ./CGImysql/sql_connection_pool.h -lpthread -lmysqlclient
clean:
		rm -r webserver
		