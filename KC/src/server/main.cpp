#include <server/server.h>

int main() {
    server::WebServer server(8080);
    server.start();
    return 0;
}
