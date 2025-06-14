#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h> // Bao gồm tệp tiêu đề libpq-fe.h

// Hàm xử lý lỗi kết nối PostgreSQL
void handle_conn_error(PGconn *conn) {
    fprintf(stderr, "Lỗi kết nối cơ sở dữ liệu: %s\n", PQerrorMessage(conn));
    PQfinish(conn); // Đóng kết nối
    exit(1);        // Thoát chương trình với mã lỗi
}

// Hàm xử lý lỗi truy vấn PostgreSQL
void handle_exec_error(PGconn *conn, PGresult *res) {
    fprintf(stderr, "Lỗi truy vấn SQL: %s\n", PQerrorMessage(conn));
    PQclear(res); // Giải phóng bộ nhớ của kết quả truy vấn
    PQfinish(conn); // Đóng kết nối
    exit(1);        // Thoát chương trình với mã lỗi
}

int main() {
    PGconn     *conn;
    PGresult   *res;
    const char *conninfo;
    const char *dbname_to_create = "mydb_from_c_app"; // Tên cơ sở dữ liệu bạn muốn tạo

    // Thông tin kết nối tới PostgreSQL server
    // Lưu ý: Để tạo database, bạn phải kết nối tới một database có sẵn (ví dụ: postgres)
    // và người dùng phải có quyền CREATE DATABASE.
    // Nếu bạn đang chạy với người dùng mặc định 'postgres' và không có mật khẩu,
    // chuỗi kết nối có thể đơn giản hơn.
    // Đối với ví dụ này, chúng ta kết nối tới database 'postgres' mặc định.
    conninfo = "host=localhost port=5432 user=postgres password=admin dbname=postgres";

    // KẾT NỐI ĐẾN MÁY CHỦ POSTGRESQL
    printf("Đang kết nối đến PostgreSQL server...\n");
    conn = PQconnectdb(conninfo);

    // Kiểm tra trạng thái kết nối
    if (PQstatus(conn) != CONNECTION_OK) {
        handle_conn_error(conn);
    }
    printf("Kết nối thành công!\n");

    // TẠO CƠ SỞ DỮ LIỆU MỚI
    // Lưu ý: Lệnh CREATE DATABASE không thể chạy trong một khối giao dịch.
    // Lệnh này không nên có "COMMIT;" hoặc "BEGIN;" xung quanh nó.
    char create_db_query[256];
    snprintf(create_db_query, sizeof(create_db_query), "CREATE DATABASE %s", dbname_to_create);

    printf("Đang tạo cơ sở dữ liệu: '%s'...\n", dbname_to_create);
    res = PQexec(conn, create_db_query);

    // Kiểm tra kết quả truy vấn
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        handle_exec_error(conn, res);
    }
    printf("Cơ sở dữ liệu '%s' đã được tạo thành công!\n", dbname_to_create);

    // Giải phóng bộ nhớ của kết quả truy vấn
    PQclear(res);

    // ĐÓNG KẾT NỐI
    PQfinish(conn);
    printf("Kết nối đã đóng. Chương trình kết thúc.\n");

    return 0; // Trả về 0 nếu thành công
}