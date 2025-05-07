"""
This code is a part of BJEV Test ToolChain
Created by ZhaoYi in 2022-08.
Copyright (c) 2022 BJEV. All rights reserved.
"""
import unittest

from fsm.env.syntax.expr_process import ExpressionInfo
from fsm.env.syntax.express import Express


class TestExpressionInfo(unittest.TestCase):
    def test_gen_atomic_expressions_info(self):
        expr = Express.make('PDCU_0x103.PDCU_RdyLamp == 1 or PDCU_0x103.PDCU_RdyLamp == 2')
        instance = ExpressionInfo(expr)
        print(instance.atomic_expressions_info)

    def test_gen_atomic_expressions_info1(self):
        expr = Express.make('PDCU_0x103.PDCU_RdyLamp > 1 or PDCU_0x103.PDCU_RdyLamp >= 2')
        instance = ExpressionInfo(expr)
        print(instance.atomic_expressions_info)

    def test_gen_atomic_expressions_info2(self):
        expr = Express.make('PDCU_0x103.PDCU_RdyLamp < 1 or PDCU_0x103.PDCU_RdyLamp <= 2')
        instance = ExpressionInfo(expr)
        print(instance.atomic_expressions_info)

    def test_gen_atomic_expressions_info3(self):
        expr = Express.make('PDCU_0x103.PDCU_RdyLamp in [1,2,"aa"] or PDCU_0x103.PDCU_RdyLamp not in [4,5,6]')
        instance = ExpressionInfo(expr)
        print(instance.atomic_expressions_info)

    def test_gen_signal_list(self):
        expr = Express.make('PDCU_0x103.PDCU_RdyLamp in [1,2,"aa"] or PDCU_0x103.PDCU_RdyLamp not in [4,5,6]')
        instance = ExpressionInfo(expr)
        print(instance.gen_signal_list())

    def test_gen_atomic_expressions_info_dict(self):
        expr = Express.make('PDCU_0x103.PDCU_RdyLamp in [1,2,"aa"] or PDCU_0x103.PDCU_RdyLamp not in [4,5,6]')
        instance = ExpressionInfo(expr)
        print(instance.gen_atomic_expression_info_dict())
