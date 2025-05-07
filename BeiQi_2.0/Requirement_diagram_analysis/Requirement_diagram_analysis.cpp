#include "graph_structures.h"

namespace fs = std::filesystem;
using namespace std;

// argv是传入的参数数组，其中默认argv[0]为程序名（无效），因此有效的参数从argv[1]开始
int main(int argc, char* argv[]) { // 通过命令行传入参数（此处是python客户端传入的参数）
    if (argc < 2) { // 没有传入有效参数
        std::cerr << "Usage: " << argv[0] << " <mode> [options]\n";
        return 1;
    }

    string mode = argv[1]; // 模式
    string port = "8080"; // 端口号
    if(argc >= 3)
        port = argv[2];
    //mode:
    // 0——c++端读取txt文档，对图进行分析、错误检测、依赖提取并将结果输出至txt文档（原本的全部功能）
    // 1——唤起c++服务器并返回服务器状态

    if (mode == "0") { // 原有功能：运行分析程序并输出文档（依赖关系、错误检测与路径字典）
        //auto start = chrono::high_resolution_clock::now();
        //ofstream out1(dependency_extract_path_loc); // 打开txt文档
        //out1.close(); // 关闭文件
        //ofstream out2(dependency_extract_graph_loc); // 打开txt文档
        //out2.close(); // 关闭文件
        //ofstream out3(output_loc); // 打开txt文档
        //out3.close(); // 关闭文件
        //ofstream out4(path_dic_loc); // 打开txt文档
        //out4.close(); // 关闭文件
        //x.initialization(); // 初始化
        //x.output_path_dic(); // 输出路径字典
        //x.dependency_extraction(); // 依赖关系提取（结果输入各个txt文档）
        //x.checkEdgeConditionUnsatisfiable(); // 边条件永不满足检测
        //x.checkGraphEdgesForSMT(); // 分支同时满足检测
        //x.graph_miss_detect(); // 需求缺失检测（包括外部需求图缺失与内部先读后写的特例）
        //x.check_dependency_cycle(); //循环依赖检测
        //x.checkForMissingBranches(); // 分支缺失检测
        //deduplicate(output_loc); // 给dependency_extract_path_loc.txt去重
        //deduplicate(dependency_extract_path_loc); // 给dependency_extract_path_loc.txt去重
        //deduplicate(dependency_extract_graph_loc); // 给dependency_extract_graph_loc.txt去重
        //auto end = chrono::high_resolution_clock::now();
        //chrono::duration<double> elapsed_seconds = end - start;
        //x.output(); // 打印到控制台
        //cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
        cout << "已弃用" << endl;
    }
    else if (mode == "1") { // 运行服务器，接收来自python的不同http通信内容并作出对应的回应
        x.run_server(x,port);
    }
    return 0;
}





