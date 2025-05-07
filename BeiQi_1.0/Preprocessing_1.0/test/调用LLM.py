import google.generativeai as genai

genai.configure(api_key="AIzaSyBPKqN5xUv6pDRxiicZ4_ExR-hJNECX1og")

# 系统指令 (system_content)
system_content = (
    "请将以下自然语言文档结构化为规定格式的图结构，包含以下要求："
    "1. **操作类型**：包括判断操作与赋值操作。"
    "2. **图结构描述**："
    "- 每个图首先要指明`graph_id` ，接下来分别表示各节点，然后表示边。"
    "3. **节点结构描述**："
    "- 每个节点由唯一的 `node_id` 标识，并包含 `node_type` 和 `node_information`。"
    "- 判断节点（`node_type`=`read`）还需要指明 `branch_yes` 和 `branch_no`，分别表示条件成立和不成立时的出边id,然后在node_information中指明要判断的条件。"
    "- 赋值节点（`node_type`=`write`）在node_information中指明赋值的操作。"
    "- 起始节点的`node_type`=`write`,`node_information`=`START`，标明流程起点。"
    "- 节点描述结束后，用node_over表示描述结束。"
    "4. **边结构描述**："
    "- 每条边用唯一的 `edge_id` 标识，并定义起始节点 `source_node_id` 和目标节点 `target_node_id`。"
    "- 如果边包含条件判断（如 `A > -10`），需额外在 `edge_information` 中指明。"
    "5. **额外说明**："
    "- 如果一个判断条件无论成立与否，都有与之对应的节点，则使用read类型节点进行判断；如果一个判断条件只在成立时有对应的节点（相当于只有yes分支），则直接用边来表示判断操作（条件成立时才能执行对应的节点）"
    "6. **示例输入、输出**："
    "输入："
    "若 A > 5，  "
    "则：B = 6  "
    "否则：B = 2  "
    "   若：A > -10  "
    "   则：B = 8  "
    "输出："
    "graph_id: d6cbccc2-4fda-11ef-b190-edbe43058cc0"
    ""
    "node_id: d6cba5b5-4fda-11ef-b190-edbe43058cc0"
    "node_type: write"
    "node_information: START"
    "node_over"
    ""
    "node_id: d6cba5b6-4fda-11ef-b190-edbe43058cc0"
    "node_type: read"
    "branch_yes: d6cba5b1-4fda-11ef-b190-edbe43058cc0"
    "branch_no: 8f6121a1-454c-4840-887e-b5aba1e99487"
    "node_information: A > 5"
    "node_over"
    ""
    "node_id: d6cba5b7-4fda-11ef-b190-edbe43058cc0"
    "node_type: write"
    "node_information: B = 6"
    "node_over"
    ""
    "node_id: d6cbccc1-4fda-11ef-b190-edbe43058cc0"
    "node_type: write"
    "node_information: B = 2"
    "node_over"
    ""
    "node_id: d6cbccc0-4fda-11ef-b190-edbe43058cc0"
    "node_type: write"
    "node_information: B = 8"
    "node_over"
    ""
    "edge_id: d6cba5b0-4fda-11ef-b190-edbe43058cc0"
    "source_node_id: d6cba5b5-4fda-11ef-b190-edbe43058cc0"
    "target_node_id: d6cba5b6-4fda-11ef-b190-edbe43058cc0"
    "edge_over"
    ""
    "edge_id: d6cba5b1-4fda-11ef-b190-edbe43058cc0"
    "source_node_id: d6cba5b6-4fda-11ef-b190-edbe43058cc0"
    "target_node_id: d6cba5b7-4fda-11ef-b190-edbe43058cc0"
    "edge_over"
    ""
    "edge_id: 8f6121a1-454c-4840-887e-b5aba1e99487"
    "source_node_id: d6cba5b6-4fda-11ef-b190-edbe43058cc0"
    "target_node_id: d6cbccc0-4fda-11ef-b190-edbe43058cc0"
    "edge_over"
    ""
    "edge_id: b1988cff-3939-4f15-85d5-b400c37b424b"
    "source_node_id: d6cbccc0-4fda-11ef-b190-edbe43058cc0"
    "target_node_id: d6cbccc1-4fda-11ef-b190-edbe43058cc0"
    "edge_information: A > -10"
    "edge_over"
    ""
    "graph_over"
)

# 用户输入 (input_text)
input_text = (
    "当以下三个条件同时满足时，进入液压驻车状态（VbEBC_PState_flg），该标志信号置1："
    "1）P挡联动标志位（VbEBC_GearPState_flg）（见EBC-9-3）为0；)"
    "2）液压驻车退出标志位（VbEBC_VehRelease_flg）为0；"
    "3）车辆静止标志位（VbEBC_VehStop_flg）由0置1的上升沿；"
    ""
    "此时若当前VCU需求扭矩补偿目标值（EBCReqTq） ≥ |驻车扭矩| +  驻车扭矩补偿值，则VCU需求扭矩补偿目标值维持不变；若当前VCU需求扭矩补偿目标值（VCU_BECTrTar） ＜ |驻车扭矩| + 驻车扭矩补偿值，则按照一定梯度上升为|驻车扭矩| +  驻车扭矩补偿值，并保持。"
    "驻车扭矩补偿值为根据驻车纵向加速度（VfEBC_LongitAccMem_Null）换算坡度查一维表（[KtEBC_ParkTqOffset_1X_null,KtEBC_ParkTqOffset_1Y_Nm]）"
    ""
    "加速踏板踩下标志位置位时，VCU需求扭矩补偿目标值随电机目标扭矩指令增加逐步降低，直至电机转速（Mot_Spd）> KfEBC_MotSpdForThrd_rpm或VCU需求扭矩补偿目标值降为0,液压驻车状态置0。"
    "当EBC-6-1中条件3）或条件4）置1或P挡联动标志位置位，液压驻车状态置0。"
)


model=genai.GenerativeModel(
  model_name="gemini-2.0-flash-exp",
  system_instruction=system_content)

# 使用变量 input_text 作为输入
response = model.generate_content(input_text)
print(response.text)