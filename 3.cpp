#define HAVE_STRUCT_TIMESPEC // ������MySQL���еĶ����ͻ
#include <windows.h>
#include <mysql.h> // ����MySQL���ͷ�ļ�
#include <opencv2/opencv.hpp> // ����OpenCV���ͷ�ļ�
#include <iostream> // ������׼�����������
#include <vector> // ������׼������
#include <fstream> // �����ļ�����




using namespace std; // ʹ�ñ�׼�����ռ�
using namespace cv; // ʹ��OpenCV�����ռ�

// ����һ���ṹ�壬���ڴ洢ͼ������
struct ImageData {
    string image_name; // ͼ������
    vector<uchar> image_data; // ͼ������
};

// ����ͼ�����ݿ�ĺ���
void insertImageToDatabase(const ImageData& imgData) {
    MYSQL* conn; // MySQL���Ӿ��
    MYSQL_STMT* stmt; // MySQL�����
    MYSQL_BIND bind[2]; // MySQL�󶨲�������

    // ��ʼ��MySQL����
    conn = mysql_init(NULL);
    if (conn == NULL) {
        cerr << "mysql_init() failed" << endl;
        return;
    }

    // ���ӵ����ݿ�
    if (mysql_real_connect(conn, "localhost", "root", "password", "student_manager", 3306, NULL, 0) == NULL) {
        cerr << "mysql_real_connect() failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    // ׼��SQL���
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

    // �󶨲���
    memset(bind, 0, sizeof(bind));

    // ��ͼ������
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)imgData.image_name.c_str();
    bind[0].buffer_length = imgData.image_name.length();

    // ��ͼ������
    bind[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[1].buffer = (char*)imgData.image_data.data();
    bind[1].buffer_length = imgData.image_data.size();

    if (mysql_stmt_bind_param(stmt, bind)) {
        cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    // ִ��SQL���
    if (mysql_stmt_execute(stmt)) {
        cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << endl;
    }
    else {
        cout << "Image inserted successfully" << endl;
    }

    // ������Դ
    mysql_stmt_close(stmt);
    mysql_close(conn);
}

void saveImageFromDB(const std::string& dbName, const std::string& tableName, const std::string& imageName, const std::string& outputPath) {
    MYSQL* conn;
    MYSQL_RES* res;
    MYSQL_ROW row;
    unsigned long* lengths; // ���ڴ洢ÿ�еĳ���  

    // ��ʼ������  
    conn = mysql_init(NULL);

    // ���ӵ����ݿ�  
    if (!mysql_real_connect(conn, "localhost", "root", "123456", dbName.c_str(), 0, NULL, 0)) {
        std::cerr << "Connection failed: " << mysql_error(conn) << std::endl;
        return;
    }

    // ׼�� SQL ��ѯ  
    std::string query = "SELECT image_data FROM " + tableName + " WHERE image_name = '" + imageName + "'";

    // ִ�в�ѯ  
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return;
    }

    // ��ȡ���  
    res = mysql_use_result(conn);

    
    // ����Ƿ�������  
    if (res) {
        row = mysql_fetch_row(res);
        if (row != NULL) {
            lengths = mysql_fetch_lengths(res); // ��ȡÿ�еĳ���  

            // �����һ���� image_data��BLOB ���ͣ�  
            std::size_t len = lengths[0]; // ʹ�õ�һ�еĳ���  
            std::vector<char> imageData(row[0], row[0] + len);

            // д���ļ�����֮ǰ��ͬ��  
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
    

    // �ر�����  
    mysql_close(conn);
}



int main() {
    /*
    // ʹ��OpenCV��ȡͼ��
    Mat image = imread("path_to_your_image.jpg", IMREAD_COLOR);
    if (image.empty()) {
        cerr << "Could not open or find the image" << endl;
        return -1;
    }

    // ��ͼ��ת��Ϊ�ֽ�����
    vector<uchar> buf;
    imencode(".jpg", image, buf);

    // ����ImageData����
    ImageData imgData;
    imgData.image_name = "example_image.jpg";
    imgData.image_data = buf;

    // ��ͼ����뵽���ݿ�
    //insertImageToDatabase(imgData);
    */
    // �����ݿ�����ȡͼ�񲢱��浽�ļ�
    saveImageFromDB("student_manager", "images", "example_image.jpg", "D:/work/C++Code/SQL/output/example_image.jpg");

    return 0;
}