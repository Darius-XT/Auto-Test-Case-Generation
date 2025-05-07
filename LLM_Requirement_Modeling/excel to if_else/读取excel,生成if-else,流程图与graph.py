import os
import json
import glob
import pandas as pd
from typing import Literal
from openai import OpenAI
import time  # 加入时间模块

# 现在生成的代码可能有顺序上的错误,比如

# ===== 选择模型 =====
# 一天5次: gpt-4o，gpt-4.1
# 一天30次: deepseek-r1, deepseek-v3
# 一天300次: gpt-4o-mini，gpt-3.5-turbo，gpt-4.1-mini，gpt-4.1-nano
ModelName = Literal["gpt-4o", "gpt-4.1", "deepseek-r1", "deepseek-v3","gpt-4o-mini", "gpt-3.5-turbo", "gpt-4.1-mini", "gpt-4.1-nano"]
current_model: ModelName = "deepseek-v3"  # 当前使用的模型

# ===== 系统提示词 =====
system_content = """
你是一个智能文档结构化助手，负责将自然语言形式的需求文档结构化，并返回对应的if-else结构语句
任务的具体要求如下：
- 按照if-else的结构化方式来理解文档：需要提取的信息只包括条件与对应的具体操作，如果一段话不为前述两者（例如是注解性质），则视为冗余语句，忽略之
- 条件一定是在判断某个变量的取值是否符合要求，而操作一定是在给某个变量赋值
- 无论是条件还是操作，都必须将原本描述性的自然语言转化为具体可执行的表达式(包括函数)
- 可能会出现对时间的约束, 此时要将其转化为函数, 规则如下:
    - duration(condition, time, sec): 当满足条件condition后，时间大于time后跳转(单位默认为sec)
    eg: 若 A = 0,且持续超过 time1 时间 → if duration(A == 0, time1, sec):
    - during(time1, time2, sec): 条件必须发生在time1与time2之间(单位默认为sec)
    eg: 若 A = 0, 且时间在 time1-time2 之间 → if A == 0 and during(time1, time2, sec):
    - after(time, sec)：条件必须发生在time1之后(单位默认为sec)
    eg: 若 A = 0, 且时间在 time1 之后 → if A == 0 and after(time1, sec):
    - before(time, sec)：条件必须发生在time1之前(单位默认为sec)
    eg: 若 A = 0, 且时间在 time1 之前 → if A == 0 and before(time1, sec):
    - 注意, duration与其它三个函数的格式不同, 它的条件写在函数括号里, 其它三个则是写为函数外的条件,用and连接
- 如果有的条件对应的操作表达的含义实际上是“不进行操作”，则将操作包括对应的条件视为冗余项，丢弃它们
- 输入文档的表述、行文风格都不固定，你需要按上述要求灵活处理各个文档
- 必须严格遵守示例中的输出规范,除了示例中的格式,不要自作主张添加或修改任何格式
以下是一个输入、输出的示例：
输入：
需求ID：State-3-1
若 A 大于 5 
则 B 的值为 6  
否则 B 为 2  
   若 A 的值超过 1  
   则将 B 赋值为 8  
输出：
if A > 5:
    B = 6
else:
    B = 2
    if A > 1:
        B = 8
"""

# ===== 配置代理,API key =====
def configure_environment():
    os.environ["http_proxy"] = "http://127.0.0.1:10809"
    os.environ["https_proxy"] = "http://127.0.0.1:10809"

    client = OpenAI(
        api_key="sk-GNNPr4Pl6gSRvbqqLgM8Y0HgZA4NdSzUdgrVuGrhhaSIW2Lu",
        base_url="https://api.chatanywhere.tech/v1"
    )
    return client

# ===== 数据处理函数 =====
def process_data_with_llm(client, model: ModelName, data_dict: dict, system_content: str):
    results = []
    total_time = 0.0  # 总耗时

    for id_value, info_value in data_dict.items():
        user_input = f"需求ID：\n{id_value}\n{info_value}"
        try:
            start_time = time.time()
            response = client.chat.completions.create(
                model=model,
                messages=[
                    {"role": "system", "content": system_content},
                    {"role": "user", "content": user_input}
                ],
                temperature=0
            )
            duration = time.time() - start_time
            total_time += duration

            llm_output = response.choices[0].message.content
        except Exception as e:
            llm_output = f"Error for ID {id_value}: {e}"
        results.append({"id": id_value, "input": info_value, "output": llm_output})

    average_time = total_time / len(data_dict) if data_dict else 0
    return results, total_time, average_time

# ===== 主函数 =====
def main():
    configure_environment()
    client = configure_environment()

    # 当前脚本所在目录
    base_dir = os.path.dirname(os.path.abspath(__file__))

    # datas 在 base_dir 的上一级目录
    parent_dir = os.path.dirname(base_dir)
    excel_dir = os.path.join(parent_dir, 'datas', 'excels')

    # 获取所有 .xlsx 文件路径（排除临时文件）
    excel_files = glob.glob(os.path.join(excel_dir, '*.xlsx'))

    # 初始化一个字典用于保存所有文件中的需求ID与内容
    data_dict = {}

    # 遍历每个 Excel 文件
    for excel_path in excel_files:
        df = pd.read_excel(
            excel_path,
            usecols=[4, 6],  # E 列和 G 列
            skiprows=range(3),  # 跳过前 3 行
        )
        # 丢弃任何包含 NaN 的行
        df = df.dropna(subset=[df.columns[0], df.columns[1]])
        # 合并到总字典中（后来的会覆盖先前的相同ID）
        file_dict = {row.iloc[0]:row.iloc[1] for _, row in df.iterrows()}
        data_dict.update(file_dict)

    # 调用模型处理数据并统计时间
    results, total_time, average_time = process_data_with_llm(client, current_model, data_dict, system_content)
    # 写入 TXT 文件
    with open("../datas/if_else_result.txt", "w", encoding="utf-8") as f:
        for entry in results:
            f.write(f"需求ID: {entry['id']}\n")
            f.write("原始输入:\n")
            f.write(entry["input"] + "\n\n")
            f.write("结构化输出:\n")
            f.write(entry["output"] + "\n")
            f.write("=" * 60 + "\n\n")

    print("✅ 结构化输出已保存到 if_else_result.txt 文件中。")
    print(f"⏱️ 总耗时: {total_time:.2f} 秒")
    print(f"⏱️ 平均每条需求耗时: {average_time:.2f} 秒")


# ===== 执行入口 =====
if __name__ == "__main__":
    main()