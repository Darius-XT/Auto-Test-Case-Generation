## 项目简介:

与北汽公司合作,对汽车领域的功能需求文档进行分析,将LLMs与确定性算法结合,端到端地自动化生成集成级别的测试用例

## 主要工作：

- 通过 few-shots 与构建领域知识库，使用 LLM 结构化需求文档, 并使用 UML 流程图保存建模结果
- 对建模结果进一步展开分析, 通过分析数据流向, 提取需求间的依赖关系, 并据此构建测试场景树
- 根据测试场景树与建模结果生成测试数据, 并应用 SMT 求解器（z3）进行求解, 生成预期测试结果
- 第一次迭代: 将核心模块（依赖关系提取）独立为 c++ 模块, 并编译为 exe 文件，通过 http 与 python 客户端进行通信，将时间开销 (以最大规模数据集为例, 提取得到共 4372 条依赖关系) 由 5min 左右降至 3s
- 第二次迭代: 改为使用 LLM 进行需求建模, 功能点覆盖率从 44% 提升到到 81%，时间开销从 3min 降低至 8.34s
- 第三次迭代: 进一步将 LLM 与算法结合，LLM 仅输出 if-else 块，再利用确定性算法将代码块转化为流程图，进一步将功能点覆盖率从 81% 提升到到 94%

**项目成果**：最终的版本已经通过北汽公司的验收测试, 将生成测试用例的时间 (小规模数据集) 由原本人工的平均
33min 降低至 20s, 同时 LLMs 对需求的建模结果准确率达 92.1%; 并实现了功能点覆盖率从 44% 到 94% 的增长

**论文链接**：（RE在投，一作）[Automated_Test_Case_Generation_in_the_Automotive_Field__An_Industry_Practice_Centered_on_Requirement_Dependencies.pdf](https://github.com/user-attachments/files/20087069/Automated_Test_Case_Generation_in_the_Automotive_Field__An_Industry_Practice_Centered_on_Requirement_Dependencies.pdf)

## 关于项目流程的详细介绍:
![图片](https://github.com/user-attachments/assets/22322656-b5bb-4e35-84bc-2c8aeaf65377)

以上为项目的主流程图：首先通过LLM进行需求建模，再通过c++模块提取数据依赖关系，最后根据依赖关系构建测试场景，并使用SMT求解器构建测试数据

需求建模的目的是对自然语言形式的文档进行结构化，并对其中的模糊描述语句进行明确；目前采用流程图来保存建模结果，如图：

![图片](https://github.com/user-attachments/assets/f088cb01-8d6d-4ceb-ac39-e2b4afabf71b)

之后通过分析流程图中路径之间的生产-消费关系进行依赖关系提取，如图：

![图片](https://github.com/user-attachments/assets/89b1a32a-3f29-4875-8912-15e8ca5f72a1)

对所有文档进行提取后，就可以将依赖关系构建为测试场景树（测试场景就表示了哪些需求之间有依赖关系，因此应当集成起来一同测试），如图：

![图片](https://github.com/user-attachments/assets/62122962-4309-4842-8469-e2788a85f8f9)

最后，根据测试场景的指示，就可以使用SMT集合所有约束并寻找可行解，并生成测试用例，如图：

![图片](https://github.com/user-attachments/assets/e02f9cad-26fc-4b89-b7b8-da55de6af256)


