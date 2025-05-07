"""
This code is a part of BJEV Test ToolChain
Created by ZhaoYi in 2022-03.
Copyright (c) 2022 BJEV. All rights reserved.
"""
from __future__ import annotations
import unittest
from fsm.env.range import Ranges
from fsm.env.range.value_interval import ValueInterval, inf
from fsm.env.range.value_set import ValueSet
from fsm.env.range.value_space import ValueSpace
from fsm.env.syntax.express import Express


class TestRange(unittest.TestCase):
    def test_value_interval1(self):
        a = ValueInterval(ValueInterval.closed(-inf, 5), ValueInterval.open(10, 100))
        b = ValueInterval(ValueInterval.closed_open(0,4), ValueInterval.open_closed(9, 99))
        print(a & b)
        print(a | b)
        print(~a)
        print(a - b)

    def test_value_interval2(self):
        value_interval_str1 = ['[1,5]', "(4,8]", "[10,100]"]
        value_interval_str2 = '''['[1,5]', "(4,8]"]'''
        value_interval_str3 = ['[0,4]']
        a = ValueInterval.from_str(value_interval_str1)
        print(isinstance(a, ValueInterval))
        b = ValueInterval.from_str(value_interval_str2)
        c = ValueInterval.from_str(value_interval_str3)
        print(c)
        print(a & b)
        print(a | b)
        print(~a)
        print(a - b)

    def test_value_set(self):
        aa=Ranges()
        value_set_str1 = '''["1",2,3,4,5,"aa","bb",get_bo,aa]'''
        value_set_str2 = '''["1",2,3,4,5,"aa","bb"]'''
        value_set_str3 = ["1",2,3,4,5,"aa",aa]
        a = ValueSet.from_str(value_set_str1)
        b = ValueSet.from_str(value_set_str2)
        c = ValueSet.from_str(value_set_str3)
        print(c)
        print(a & b)
        print(a | b)
        print(a - b)
        print(b & c)
        print(c - b)
        print(b - c)

    def test_value_space(self):
        aa = Ranges()
        value_interval_str = ['[1,5]', "(4,8]", "[10,100]"]
        value_set_str1 = '''["1",2,3,4,5,"aa","bb"]'''
        value_set_str2 = ["1", 2, 3, 4, 5,"aa","bb", aa]
        value_set_str3 = '''["1",2,3,4,5,"aa","bb",aa]'''
        a = ValueSpace.from_str(value_interval_str, value_set_str1)
        b = ValueSpace.from_str(value_interval_str, value_set_str2)
        print(a & b)
        print(a | b)
        print(a - b)
        print(b - a)
        c = ValueSpace.from_str(value_interval_str, value_set_str3)
        print(c)

    def test_value_space1(self):
        value_interval_str = ['[1,5]', "(4,8]", "[10,100]"]
        value_set_str1 = '''["1",2,3,4,5,"aa","bb"]'''
        a = ValueSpace.from_atomic(ValueInterval(), ValueSet())
        b = ValueSpace.from_atomic(ValueInterval.from_str(value_interval_str), ValueSet.from_str(value_set_str1))
        print(a)
        print(b)
        print(a & b)
        print(a | b)
        # c = ValueSpace.from_atomic(ValueInterval(), None)
        # print(c)
