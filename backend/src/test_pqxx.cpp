#include <pqxx/pqxx>
#include <iostream>

int main() {
    try {
        // Replace these values with your actual PostgreSQL/RDS details
        std::string conn_str = "host=database-users.cpaycuiokmmm.eu-north-1.rds.amazonaws.com user=postgres password=i7xjabmVeQMgUbfR4IPC dbname=postgres";
        pqxx::connection c(conn_str);
        pqxx::work txn(c);
        pqxx::result r = txn.exec("SELECT version();");
        std::cout << "PostgreSQL version: " << r[0][0].c_str() << std::endl;
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
