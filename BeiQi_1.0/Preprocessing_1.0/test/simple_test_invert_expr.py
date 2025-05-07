"""
This code is a part of BJEV Test ToolChain
Created by ZhaoYi in 2022-08.
Copyright (c) 2022 BJEV. All rights reserved.
"""
import unittest
from fsm.env.syntax.express import Express


class TestExpr(unittest.TestCase):
    def test_simple_atomic_event_invert(self):
        code = 'a'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_event_invert(self):
        code = 'a and b'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_gt_invert(self):
        code = 'a>0'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_ge_invert(self):
        code = 'a>=0'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_lt_invert(self):
        code = 'a<0'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_le_invert(self):
        code = 'a<=0'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_eq_invert(self):
        code = 'a==0'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_not_eq_invert(self):
        code = 'a!=0'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_in_invert(self):
        code = 'a in [0,1]'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_not_in_invert(self):
        code = 'a not in [0,1]'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_function_call_invert(self):
        code = 'foo(x, y)'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_express_invert(self):
        code = 'a>0 and c<10 and d==5 or foo(x)'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())

    def test_simple_atomic_express_invert1(self):
        code = 'after(3,sec)'
        express = Express.make(code)
        invert_express = ~express
        print(invert_express.unparse())
