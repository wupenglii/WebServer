#include <mysql/mysql.h>
#include <iostream>
#include <string>
#include <string.h>
#include <cstdio>
#include "sql_connection_pool.h"
#include <map>
#include <fstream>
#include <sstream>
using namespace std;

//#define CGISQL         //不使用连接池
#define CGISQLPOOL  //使用连接池

int main(int argc, char *argv[]){

    locker lock;
#ifdef CGISQL
    MYSQL *con = NULL;
    con = mysql_init(con);

    if(con == NULL){
        cout <<"Error:"<<mysql_error(con);
        exit(1);
    }

    con = mysql_real_connect(con,"localhost","root","123","dataDb",3306,NULL,0);

    if(con == NULL){
        cout <<"Error: "<<mysql_error(con);
    }

    if(mysql_query(con,"SELECT goodid,goodname,height,length,width FROM goods")){
        printf("INSERT error:%s\n",mysql_error(con));
        return -1;
    }
    //从表中检索完整的结果集
    MYSQL_RES * result = mysql_store_result(con);
    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);
    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    string goodid = "123";
    string goodname = "变压器";
    string height = "12";
    string length = "12";
    string width = "12";
    char *sql_insert = (char *)malloc(sizeof(char)*200);
    strcpy(sql_insert,"INSERT INTO goods(goodid,goodname,height,length,width)VALUES(");
    strcat(sql_insert,"'");
    strcat(sql_insert,goodid.c_str());
    strcat(sql_insert,"', '");
    strcat(sql_insert,goodname.c_str());
    strcat(sql_insert,"', '");
    strcat(sql_insert,height.c_str());
    strcat(sql_insert,"', '");
    strcat(sql_insert,length.c_str());
    strcat(sql_insert,"', '");
    strcat(sql_insert,width.c_str());
    strcat(sql_insert,"');");

    lock.lock();
    int res = mysql_query(con,sql_insert);
    lock.unlock();

    if(!res){
        printf("1\n");
    }else{
        printf("0\n");
    }
    mysql_free_result(result);

#endif

    return 0;
}