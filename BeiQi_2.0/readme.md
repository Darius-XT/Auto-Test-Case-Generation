# **文件说明：**

## **Preprocessing**

代码类型：python

功能：解析需求图文件，并可调用c++进行图文件的更新与依赖关系、路径的请求

参数：与graph_parse.py相同

使用方式：（http_connect.py）

功能均实现在http_communication中，以下为各函数说明：

1. setUp——初始化

   参数说明：

   - port：与c++进行http通信的端口
   - cpp_server_path：对应的c++可执行文件（exe）的路径

2. test_call_cpp_server——唤起c++服务器，成功则返回提示（如果这个函数进程被终止，服务器会随之终止）

3. test_send_graph——向c++服务器传入图信息（传递所有图or传递部分图均可）

   参数说明：

   - solution_path = "..\\demo_分支_环路_输出"（不变）
   - graph_ids = "5915518c-2c46-11ef-97ca-c8cb9e4ad5d4" （只更新部分图时才有意义，多个图则用逗号分隔）
   - env_name = "default"（不变）
   - database_id = "c263f6cb-c1e3-4f04-a714-fa54b58abbca-default"（不变）
   - modify_all = True （True代表全部传入，False则只传入graph_ids中的部分）

4. test_ask_requirements——请求服务器返回所有依赖关系

5. test_ask_pathdic——请求服务器返回所有路径信息

## **Requirement_diagram_analysis**

代码类型：C++

功能：通过命令行参数控制，mode = 0则仍然执行1.0版本相关功能（已弃用），mode = 1则作为一个服务器被调用

如果只是运行相关功能，这个文件夹其实已经没有必要，但“**Requirement_diagram_analysis.exe**”文件是由这个c++ project编译得到的，因此如果丢失，可以通过重新编译来得到，也可以由此查看c++的具体实现逻辑

main函数接口：（与代码运行无关，仅作说明）

- mode = 0：c++端读取txt文档，对图进行分析、错误检测、依赖提取并将结果输出至txt文档（原本的全部功能）
- mode = 1：唤起c++服务器（http）
  - mode = a：传入图，更新数据结构
  - mode = b：要求返回依赖关系
  - mode = c：要求返回路径信息

## **Requirement_diagram_analysis.exe**

已经被编译为可执行文件的c++代码，通过python客户端进行调用，可进行图文件的更新与依赖关系、路径的返回

