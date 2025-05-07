from typing import List

from data_structure.requirement_structure import Requiements

# 主执行入口
if __name__ == "__main__":
    # 将文件读取,存入数据结构
    r_s = Requiements()

    r_s.read_file("../datas/if_else_result.txt")
    r_s.generate_flowchart_code()
    r_s.generate_graph_code()
    r_s.output_flowcharts()
    # r_s.print_graph_to_console()
    r_s.output_graphs()




