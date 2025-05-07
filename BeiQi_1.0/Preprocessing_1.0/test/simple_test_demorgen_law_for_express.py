"""
This code is a part of BJEV Test ToolChain
Created by Zhaoyi in 2022-08.
Copyright (c) 2022 BJEV. All rights reserved.
"""
import unittest
from fsm.env.syntax.de_morgen_law import DeMorgenLawForExpress, Express


class TestDeMorgenLaw(unittest.TestCase):
    def test_atomic_compare_gt(self):
        expr = 'not a>1'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_compare_gte(self):
        expr = 'not a>=1'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_compare_lt(self):
        expr = 'not a<1'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_compare_lte(self):
        expr = 'not a<=1'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_eq(self):
        expr = 'not a==1'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_not_eq(self):
        expr = 'not a!=1'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_in(self):
        expr = 'not a in [1,2]'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_not_in(self):
        expr = 'not a not in [1,2]'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_event(self):
        expr = 'not a'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_call(self):
        expr = 'not a()'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_bool_op_and(self):
        expr = 'not (a>1 and b<5)'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_atomic_bool_op_or(self):
        expr = 'not (a>1 or b<5)'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_complex_expression1(self):
        expr = 'not (a>1 and b>1) or not(c>10 or d>5)'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_complex_expression2(self):
        expr = 'not (a>1 and b>1) or not(c>10 or d>5) and e<6'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_complex_expression3(self):
        expr = 'not (a>1 and (b>1 or c>1))'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())

    def test_complex_expression_without_not(self):
        expr = 'a>1 and b>1 or c>10 or d>5 and e<6'
        de_morgen_law = DeMorgenLawForExpress()
        expr_processed = Express.make(expr)
        res = de_morgen_law(expr_processed)
        print(res.unparse())
