"""
This code is a part of BJEV Test ToolChain
Created by ZhaoYi in 2022-03.
Copyright (c) 2022 BJEV. All rights reserved.
"""
import unittest
from fsm.test_coverage.coverage_method_new import CoverageMethodGeneratorReq
from fsm.config.coverage_method_config import CoverageMethodConfigReq
from fsm.config.usage_error_message_config import UsageErrorMessageConfig
from test_data.simple_test_coverage_method_config import simple_coverage_method_data_req
from fsm.env.syntax.expr_process import ConditionExprProcess
from fsm.fsm_fsm import FSM
from test_data.simple_project_demo import simple_project_demo
from fsm.env.environment_runtime import RuntimeNamespace
from fsm.solution import Solution
from fsm.graph.time_factor import TimeFactor

import tracemalloc
tracemalloc.start()
def get_fsm_from_solution(solution_path, env_name, database_id):
    solution = Solution(solution_path, database_id)
    if not solution.check_valid():
        solution = None
    if solution is None:
        print('Cannot load the solution folder from:' + solution_path)
        return None
    env = solution.env_config.select_env(env_name)
    graphs = solution.get_all_graph_list()
    local_runtime = {}
    env.global_runtime = env.runtime.shallow_copy()
    for graph_item in graphs:
        env.local_runtime[graph_item.id] = RuntimeNamespace()
        local_runtime_item = env.local_runtime[graph_item.id].gen_local_runtime_item(graph_item, env)
        local_runtime[graph_item.id] = local_runtime_item
    env.local_runtime = local_runtime
    if env is None:
        print('Failed to open env:' + env_name)
        return None
    if graphs is None:
        print('Failed to get graph list')
        return None
    return FSM(graphs, env)


class TestCoverageMethod(unittest.TestCase):
    def test_simple_coverage_after(self):
        solution_path = '..\demo_分支_环路_输出'
        env_name = 'default'
        database_id = 'c263f6cb-c1e3-4f04-a714-fa54b58abbca-default'
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        expr = 'after(6,sec)'
        expr1 = ConditionExprProcess(fsm, fsm.graphs[0], expr, UsageErrorMessageConfig("EN"))
        coverage_method_config = CoverageMethodConfigReq(simple_coverage_method_data_req)
        result = CoverageMethodGeneratorReq(expr1, fsm.env, coverage_method_config, TimeFactor.new())
        result1 = result.get_bool_value_combinations()
        result2 = result.get_value_space_combinations()
        print(result1)
        print(result2)

    def test_simple_coverage_before(self):
        solution_path = '..\demo_分支_环路_输出'
        env_name = 'default'
        database_id = 'c263f6cb-c1e3-4f04-a714-fa54b58abbca-default'
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        expr = 'before(6,sec) and 钥匙开关==2'
        expr1 = ConditionExprProcess(fsm, fsm.graphs[0], expr, UsageErrorMessageConfig("EN"))
        coverage_method_config = CoverageMethodConfigReq(simple_coverage_method_data_req)
        result = CoverageMethodGeneratorReq(expr1, fsm.env, coverage_method_config, TimeFactor.new())
        result1 = result.get_bool_value_combinations()
        result2 = result.get_value_space_combinations()
        print(result1)
        print(result2)

    def test_simple_coverage_during(self):
        solution_path = '..\demo_分支_环路_输出'
        env_name = 'default'
        database_id = 'c263f6cb-c1e3-4f04-a714-fa54b58abbca-default'
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        expr = 'during(6,10,sec) and 钥匙开关==2'
        expr1 = ConditionExprProcess(fsm, fsm.graphs[0], expr, UsageErrorMessageConfig("EN"))
        coverage_method_config = CoverageMethodConfigReq(simple_coverage_method_data_req)
        result = CoverageMethodGeneratorReq(expr1, fsm.env, coverage_method_config, TimeFactor.new())
        result1 = result.get_bool_value_combinations()
        result2 = result.get_value_space_combinations()
        print(result1)
        print(result2)

    def test_simple_coverage_duration(self):
        solution_path = '..\demo_分支_环路_输出'
        env_name = 'default'
        database_id = 'c263f6cb-c1e3-4f04-a714-fa54b58abbca-default'
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        expr = 'duration(钥匙开关==2,10,sec)'
        expr1 = ConditionExprProcess(fsm, fsm.graphs[0], expr, UsageErrorMessageConfig("EN"))
        coverage_method_config = CoverageMethodConfigReq(simple_coverage_method_data_req)
        result = CoverageMethodGeneratorReq(expr1, fsm.env, coverage_method_config, TimeFactor.new())
        result1 = result.get_bool_value_combinations()
        result2 = result.get_value_space_combinations()
        print(result1)
        print(result2)

    def test_simple_coverage_method(self):
        solution_path = '..\demo_分支_环路_输出'
        env_name = 'default'
        database_id = 'c263f6cb-c1e3-4f04-a714-fa54b58abbca-default'
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        expr = '(duration(BMS自检计数器==1,3,sec))'
        expr1 = ConditionExprProcess(fsm, fsm.graphs[0], expr, UsageErrorMessageConfig("EN"))
        coverage_method_config = CoverageMethodConfigReq(simple_coverage_method_data_req)
        result = CoverageMethodGeneratorReq(expr1, fsm.env, coverage_method_config, TimeFactor.new())
        result1 = result.get_bool_value_combinations()
        result2 = result.get_value_space_combinations()
        print(result1)
        print(result2)

    def test_simple_coverage_method2(self):
        solution_path = '..\demo_分支_环路_输出'
        env_name = 'default'
        database_id = 'c263f6cb-c1e3-4f04-a714-fa54b58abbca-default'
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        expr = 'BMS自检计数器==1 and (during(5,10,sec) or during(9,14,sec))'
        expr1 = ConditionExprProcess(fsm, fsm.graphs[0], expr, UsageErrorMessageConfig("EN"))
        coverage_method_config = CoverageMethodConfigReq(simple_coverage_method_data_req)
        result = CoverageMethodGeneratorReq(expr1, fsm.env, coverage_method_config, TimeFactor.new())
        result1 = result.get_bool_value_combinations()
        result2 = result.get_value_space_combinations()
        print(result1)
        print(result2)

    def test_simple_coverage_method5(self):
        solution_path = '..\demo_分支_环路_输出'
        env_name = 'default'
        database_id = 'c263f6cb-c1e3-4f04-a714-fa54b58abbca-default'
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        expr = 'BMS自检计数器==1 and 整车故障立即下高压开关==1 and after(6,sec)'
        expr1 = ConditionExprProcess(fsm, fsm.graphs[0], expr, UsageErrorMessageConfig("EN"))
        coverage_method_config = CoverageMethodConfigReq(simple_coverage_method_data_req)
        result = CoverageMethodGeneratorReq(expr1, fsm.env, coverage_method_config, TimeFactor.new())
        result1 = result.get_bool_value_combinations()
        result2 = result.get_value_space_combinations()
        print(result1)
        print(result2)

    def test_simple_coverage_method6(self):
        solution_path = '..\demo_分支_环路_输出'
        env_name = 'default'
        database_id = 'c263f6cb-c1e3-4f04-a714-fa54b58abbca-default'
        fsm = get_fsm_from_solution(solution_path, env_name, database_id)
        expr = 'BMS自检计数器==1 and 整车故障立即下高压开关==1 and (整车模式==2 or 整车模式==7 or 整车模式==8 or 整车模式==10)'
        expr1 = ConditionExprProcess(fsm, fsm.graphs[0], expr, UsageErrorMessageConfig("EN"))
        coverage_method_config = CoverageMethodConfigReq(simple_coverage_method_data_req)
        result = CoverageMethodGeneratorReq(expr1, fsm.env, coverage_method_config, TimeFactor.new())
        result1 = result.get_bool_value_combinations()
        result2 = result.get_value_space_combinations()
        print(result1)
        print(result2)
