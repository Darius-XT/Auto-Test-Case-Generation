import time

from .simple_adder_data import adder_data

g1 = {
    'id': 'g1',
    'desc': '121',
    'type_name': 'graph',
    'h_function': 'none',
    'entry_action_list': [],
    'exit_action_list': [],
    'nodes': [
        {
            'type_name': 'state',
            'id': 'g1_start',
            'name': 'g1 Start',
            'desc': 'g1 Start',
            'is_init_node': True,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': [],
            'output_transitions': ['g1_event_next']
        },
        {
            'type_name': 'state',
            'ref_graph': 'g2',
            'id': 'g1_call_g2',
            'name': 'g1 calc',
            'desc': 'g1 calc',
            'is_init_node': False,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['g1_event_next'],
            'output_transitions': ['g1_event_finish']
        },
        {
            'type_name': 'state',
            'id': 'g1_finish',
            'name': 'g1 Finish',
            'desc': 'g1 Finish',
            'is_init_node': False,
            'is_finish_node': True,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['g1_event_finish'],
            'output_transitions': []
        },
    ],
    'transitions': [
        {
            'type_name': 'transition',
            'id': 'g1_event_next',
            'name': 'start',
            'desc': '',
            'condition': '',
            'event_list': [],
            'action_list': [],
            'source_node': 'g1_start',
            'target_node': 'g1_call_g2',
            'weights': {'value': 1}
        },
        {
            'type_name': 'transition',
            'id': 'g1_event_finish',
            'name': 'g1_finish',
            'desc': '',
            'condition': '',
            'event_list': [],
            'action_list': [],
            'source_node': 'g1_call_g2',
            'target_node': 'g1_finish',
            'weights': {'value': 1}
        },
    ]
}

g2 = {
    'id': 'g2',
    'desc': 'sum adder',
    'type_name': 'graph',
    'h_function': 'none',
    'entry_action_list': [],
    'exit_action_list': [],
    'nodes': [
        {
            'type_name': 'state',
            'id': 'g2_start',
            'name': 'Start',
            'desc': 'Start',
            'is_init_node': True,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': [],
            'output_transitions': ['g2_calc']
        },
        {
            'type_name': 'state',
            'id': 'g2_adder',
            'name': 'adder',
            'desc': 'Running',
            'is_init_node': False,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'sum=sum+n', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0},
                {'express': 'n=n-1', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [
                {'express': 'print(f"sum = {sum} n={n}")', 'type': 'action', 'pre_think_time': 0,
                 'post_think_time': 0}],
            'input_transitions': ['g2_calc', 'g2_continue'],
            'output_transitions': ['g2_end', 'g2_continue']
        },
        {
            'type_name': 'state',
            'id': 'g2_finish',
            'name': 'finish',
            'desc': 'Finish',
            'is_init_node': False,
            'is_finish_node': True,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'print(f"Finish node says: Result of sum is {sum}")',
                 'type': 'action',
                 'pre_think_time': 0,
                 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['g2_end'],
            'output_transitions': []
        },
    ],
    'transitions': [
        {
            'type_name': 'transition',
            'id': 'g2_calc',
            'name': 'start',
            'desc': 'goto adder',
            'condition': '',
            'event_list': [],
            'action_list': [],
            'source_node': 'g2_start',
            'target_node': 'g2_adder',
            'weights': {'value': 1}
        },
        {
            'type_name': 'transition',
            'id': 'g2_continue',
            'name': 'continue',
            'desc': 'goto adder',
            'condition': 'sum>0',
            'event_list': [],
            'action_list': [],
            'source_node': 'g2_adder',
            'target_node': 'g2_adder',
            'weights': {'value': 1},
            'test_layer': {'max_entry_count': 5}
        },
        {
            'type_name': 'transition',
            'id': 'g2_end',
            'name': 'end',
            'desc': 'end',
            'condition': 'n==0',
            'event_list': [],
            'action_list': [],
            'source_node': 'g2_adder',
            'target_node': 'g2_finish',
            'weights': {'value': 1}
        }
    ]
}

simple_project = {
    'name': 'test project',
    'desc': [],
    'creator': 'bolow',
    'create_time': round(time.time() * 1000000),
    'updator': 'bolow',
    'update_time': round(time.time() * 1000000),
    'environments': [{'types': adder_data, 'logics': []}],
    'graphs': [g1, g2]
}
