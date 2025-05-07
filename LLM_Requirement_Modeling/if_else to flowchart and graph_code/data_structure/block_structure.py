import re

# 语句类,为每个语句存储其父亲语句与孩子语句,即存储语句间的嵌套关系
class Statement:
    def __init__(self, statement_id: int, statement_type: str, content: str, level: int):
        self.statement_id = statement_id
        self.statement_type = statement_type  # 'if', 'elif', 'else', 'action'
        self.content = content.strip() # 语句内容
        self.level = level # 语句嵌套层数(从0开始)
        self.children: List["Statement"] = [] # 孩子
        self.parent: Optional["Statement"] = None # 父亲

    # 以树形结构输出当前的各个语句
    def print_statement(self, indent=0):
        indent_str = "  " * indent
        print(f"{indent_str}[{self.statement_id}] {self.statement_type.upper()} | Level: {self.level} | Content: {self.content}")
        if self.parent:
            print(f"{indent_str}  └─ Parent ID: [{self.parent.statement_id}]")
        if self.children:
            print(f"{indent_str}  └─ Children: {[child.statement_id for child in self.children]}")
            for child in self.children:
                child.print_statement(indent + 2)

# 语句块类:存储所有if-else语句
class Block:
    # 传入对应的if-else块
    def __init__(self, block_content):
        self.block_content = block_content
        self.statements: List[Statement] = []  # 语句列表

        # 构造Block时, 就根据content构造block中的statements
        self.parse_statements()

    # 为语句列表添加语句
    def add_statement(self, statement: Statement):
        self.statements.append(statement)

    # 根据 self.block_content 构造 statements 并存入 self.statements
    def parse_statements(self):

        # 内部函数,根据整行内容与keyword(if/elif)提取statement中的content
        def parse_condition(stripped: str, keyword: str) -> str:
            """内部小函数：根据关键字解析条件内容"""
            after = stripped[len(keyword):]
            after = after.rstrip(":")

            if after.startswith(" "):
                content = after.strip()
            elif after.startswith("(") and after.endswith(")"):
                content = after[1:-1].strip()
            else:
                content = after.strip()

            return content

        # 内部函数, 判断给定字符串所处的嵌套层数
        def get_indent_level(line: str, tabsize=4) -> int:
            stripped = line.lstrip()
            return (len(line) - len(stripped)) // tabsize

        lines = self.block_content.strip().split("\n")
        stack = []
        local_statement_id = 0  # 每个图中的 statement_id 从 0 开始\

        for line in lines:
            level = get_indent_level(line)
            stripped = line.strip()

            if not stripped:
                continue  # 跳过空行

            # 创建语句
            if stripped.startswith("if"):
                content = parse_condition(stripped, "if")
                statement = Statement(local_statement_id, "if", content, level)

            elif stripped.startswith("elif") or stripped.startswith("else if"):
                content = parse_condition(stripped, "elif")
                statement = Statement(local_statement_id, "elif", content, level)

            elif stripped.startswith("else if"):
                content = parse_condition(stripped, "else if")
                statement = Statement(local_statement_id, "elif", content, level)

            elif stripped.startswith("else"):
                statement = Statement(local_statement_id, "else", "otherwise", level)

            # 不含任何前缀, 直接当 action 语句
            else:
                statement = Statement(local_statement_id, "action", stripped, level)

            local_statement_id += 1

            # 建立父子关系
            while stack and stack[-1].level >= level:
                stack.pop()

            if stack:
                parent = stack[-1]
                statement.parent = parent
                parent.children.append(statement)

            stack.append(statement)
            self.add_statement(statement)

        # 创建 start 语句（最顶层语句的共同父语句）
        start_statement = Statement(local_statement_id, "start", "Start", -1)
        local_statement_id += 1
        for statement in self.statements:
            if statement.level == 0:
                statement.parent = start_statement
                start_statement.children.append(statement)
        self.add_statement(start_statement)

        # 创建 end 语句（没有子语句,且当前语句为父语句的最后一个孩子,此时令其子语句为end）
        end_statement = Statement(local_statement_id, "end", "End", max(n.level for n in self.statements if not n.children) + 1)
        for statement in self.statements:
            if not statement.children and statement.statement_type != "end":  # 忽略 end 自己
                # 检查当前语句是否为它父亲的最后一个孩子
                if statement.parent and statement.parent.children[-1] == statement:
                    statement.children.append(end_statement)
                    end_statement.parent = statement  # 最后一个被连接的 statement 为 parent（也可以不设）
        self.add_statement(end_statement)

    # 打印图中的所有语句内容与层次信息
    def print_block(self):
        root_statements = [n for n in self.statements if n.parent is None]
        for root in root_statements:
            root.print_statement()

    # 打印图的if-else代码与对应的流程图代码
    def print_block_with_flowchart(self):
        print("\n流程图代码:")
        if not self.flowchart_code:
            self.generate_flowchart_code()
        print(self.flowchart_code)

    # 把图的if-else代码与对应的流程图代码保存到txt
    def save_block_with_flowchart(self, f):
        # 图id
        # 保存流程图代码
        f.write("```mermaid\n")
        f.write(self.flowchart_code)
        f.write("\n```\n\n")