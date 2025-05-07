"""
This code is a part of BJEV Test ToolChain
Created by Bolow in 2022-02.
Copyright (c) 2022 BJEV. All rights reserved.
"""
simple_graph_config = {
    'id': 'main-graph',
    'desc': 'test is a main graph',
    'type_name': 'graph',
    'h_function': 'none',
    'entry_action_list': [],
    'exit_action_list': [],
    'nodes': [
        {
            'type_name': 'state',
            'id': 'state-001',
            'name': 'stateA',
            'desc': 'init',
            'is_init_node': True,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': [],
            'output_transitions': ['event-001']
        },
        {
            'type_name': 'state',
            'id': 'state-002',
            'name': 'stateB',
            'desc': 'Running',
            'is_init_node': False,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['event-001'],
            'output_transitions': ['event-002']
        },
        {
            'type_name': 'state',
            'id': 'state-003',
            'name': 'stateC',
            'desc': 'Finish',
            'is_init_node': False,
            'is_finish_node': True,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['event-002'],
            'output_transitions': ['event-003']
        },
    ],
    'transitions': [
        {
            'type_name': 'transition',
            'id': 'event-001',
            'name': 'start',
            'desc': 'goto run',
            'condition': '',
            'event_list': ['start'],
            'action_list': [],
            'source_node': 'state-001',
            'target_node': 'state-002',
            'weights': {'value': 1}
        },
        {
            'type_name': 'transition',
            'id': 'event-002',
            'name': 'finish',
            'desc': 'goto finish',
            'condition': '',
            'event_list': ['stop'],
            'action_list': [],
            'source_node': 'state-002',
            'target_node': 'state-003',
            'weights': {'value': 1}
        }
    ]
}
