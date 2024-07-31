#define HAVE_STRUCT_TIMESPEC // 避免与MySQL库中的定义冲突
#include <windows.h>
#include <mysql.h> // 包含MySQL库的头文件
#include <opencv2/opencv.hpp> // 包含OpenCV库的头文件
#include <iostream> // 包含标准输入输出流库
#include <vector> // 包含标准向量库
#include <fstream> // 包含文件流库




using namespace std; // 使用标准命名空间
using namespace cv; // 使用OpenCV命名空间

// 定义一个结构体，用于存储图像数据
struct ImageData {
    string image_name; // 图像名称
    vector<uchar> image_data; // 图像数据
};

// 插入图像到数据库的函数
void insertImageToDatabase(const ImageData& imgData) {
    MYSQL* conn; // MySQL连接句柄
    MYSQL_STMT* stmt; // MySQL语句句柄
    MYSQL_BIND bind[2]; // MySQL绑定参数数组

    // 初始化MySQL连接
    conn = mysql_init(NULL);
    if (conn == NULL) {
        cerr << "mysql_init() failed" << endl;
        return;
    }

    // 连接到数据库
    if (mysql_real_connect(conn, "localhost", "root", "password", "student_manager", 3306, NULL, 0) == NULL) {
        cerr << "mysql_real_connect() failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    // 准备SQL语句
    const char* query = "INSERT INTO images (image_name, image_data) VALUES (?, ?)";
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        cerr << "mysql_stmt_init() failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    // 绑定参数
    memset(bind, 0, sizeof(bind));

    // 绑定图像名称
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)imgData.image_name.c_str();
    bind[0].buffer_length = imgData.image_name.length();

    // 绑定图像数据
    bind[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[1].buffer = (char*)imgData.image_data.data();
    bind[1].buffer_length = imgData.image_data.size();

    if (mysql_stmt_bind_param(stmt, bind)) {
        cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    // 执行SQL语句
    if (mysql_stmt_execute(stmt)) {
        cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << endl;
    }
    else {
        cout << "Image inserted successfully" << endl;
    }

    // 清理资源
    mysql_stmt_close(stmt);
    mysql_close(conn);
}

void saveImageFromDB(const std::string& dbName, const std::string& tableName, const std::string& imageName, const std::string& outputPath) {
    MYSQL* conn;
    MYSQL_RES* res;
    MYSQL_ROW row;
    unsigned long* lengths; // 用于存储每列的长度  

    // 初始化连接  
    conn = mysql_init(NULL);

    // 连接到数据库  
    if (!mysql_real_connect(conn, "localhost", "root", "123456", dbName.c_str(), 0, NULL, 0)) {
        std::cerr << "Connection failed: " << mysql_error(conn) << std::endl;
        return;
    }

    // 准备 SQL 查询  
    std::string query = "SELECT image_data FROM " + tableName + " WHERE image_name = '" + imageName + "'";

    // 执行查询  
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return;
    }

    // 获取结果  
    res = mysql_use_result(conn);

    
    // 检查是否有数据  
    if (res) {
        row = mysql_fetch_row(res);
        if (row != NULL) {
            lengths = mysql_fetch_lengths(res); // 获取每列的长度  

            // 假设第一列是 image_data（BLOB 类型）  
            std::size_t len = lengths[0]; // 使用第一列的长度  
            std::vector<char> imageData(row[0], row[0] + len);

            // 写入文件（与之前相同）  
            std::ofstream file(outputPath, std::ios::binary);
            if (file.is_open()) {
                file.write(imageData.data(), imageData.size());
                file.close();
                std::cout << "Image saved to " << outputPath << std::endl;
            }
            else {
                std::cerr << "Failed to open file for writing" << std::endl;
            }
        }
        mysql_free_result(res);
    }
    

    // 关闭连接  
    mysql_close(conn);
}



int main() {
    /*
    // 使用OpenCV读取图像
    Mat image = imread("path_to_your_image.jpg", IMREAD_COLOR);
    if (image.empty()) {
        cerr << "Could not open or find the image" << endl;
        return -1;
    }

    // 将图像转换为字节数组
    vector<uchar> buf;
    imencode(".jpg", image, buf);

    // 创建ImageData对象
    ImageData imgData;
    imgData.image_name = "example_image.jpg";
    imgData.image_data = buf;

    // 将图像插入到数据库
    //insertImageToDatabase(imgData);
    */
    // 从数据库中提取图像并保存到文件
    saveImageFromDB("student_manager", "images", "example_image.jpg", "D:/work/C++Code/SQL/output/example_image.jpg");

    return 0;
}