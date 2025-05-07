"""
This code is a part of BJEV Test ToolChain
Created by Bolow in 2022-03.
Copyright (c) 2022 BJEV. All rights reserved.
"""
import unittest

from fsm.env.syntax.expr_tool import ValueNameCollector, FunctionNameCollector
from fsm.env.syntax.express import Express


class TestExpr(unittest.TestCase):
    def test_simple_express(self):
        code = 'a>0 and c<10 and d==5 or foo(x)'
        express = Express.make(code)
        self.assertTrue(express is not None)
        print(express)
        return express

    def test_defect_23(self):
        code = '(慢充==1 and b>=1) or (f==2 and c in [0.5,5]) and d==6 or e==7'
        express = Express().make(code)
        self.assertTrue(express is not None)

    def test_wrong_express(self):
        try:
            code = 'a >b'
            express = Express.make(code)
        except SyntaxError as e:
            print(str(e))

    def test_name_collector(self):
        code = 'a>0 and c<10 and d==5 or foo(x)'
        value_name_collector = ValueNameCollector(code)
        name_list = value_name_collector.run()
        self.assertEqual(name_list, ['a', 'c', 'd', 'x'])

    def test_function_collector(self):
        code = 'f1()>0 and b<f2() and f2()==5 or f3(f4(),f5(f6()))'
        function_name_collector = FunctionNameCollector(code)
        function_list = function_name_collector.run()
        self.assertEqual(function_list, ['f1', 'f2', 'f3', 'f4', 'f5', 'f6'])

    def test_not_express(self):
        code = 'not a>1'
        express = Express().make(code)
        print(express)
        print(express.unparse())

    def test_not_express1(self):
        code = 'not (a>1 and b>1)'
        express = Express().make(code)
        print(express)
        print(express.unparse())

    def test_not_express2(self):
        code = 'not a>1 and b>1'
        express = Express().make(code)
        print(express)
        print(express.unparse())

    def test_not_express3(self):
        code = 'not 1'
        express = Express().make(code)
        print(express)
        print(express.unparse())

    def test_not_express4(self):
        code = 'not call(test1)'
        express = Express().make(code)
        print(express)
        print(express.unparse())
