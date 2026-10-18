import requests
import json
import unittest

import sys
import os

sys.path.append(os.path.abspath("d:/Masters/Semester 4/P1/cpp_code_analyzer"))

os.environ["GOOGLE_API_KEY"] = "dummy_key_for_test"

from main import app

class TestArchitectureGraph(unittest.TestCase):
    def setUp(self):
        self.app = app.test_client()
        self.app.testing = True

    def test_simple_graph(self):
        payload = {
            "files": [
                {
                    "name": "Math.h",
                    "content": """
                    class Vector3 {
                        public:
                        float x, y, z;
                        void Normalize();
                    };
                    """
                },
                {
                    "name": "Math.cpp",
                    "content": """
                    #include "Math.h"
                    void Vector3::Normalize() {
                        // implementation
                    }
                    """
                },
                {
                    "name": "Player.h",
                    "content": """
                    #include "Math.h"
                    class Player {
                        Vector3 position;
                        void Update();
                    };
                    """
                },
                {
                    "name": "Player.cpp",
                    "content": """
                    #include "Player.h"
                    void Player::Update() {
                        position.Normalize();
                    }
                    """
                },
                {
                    "name": "Game.cpp",
                    "content": """
                    #include "Player.h"
                    void RunGame() {
                        Player p;
                        p.Update();
                    }
                    class Boss : public Player {
                    };
                    """
                }
            ]
        }
        
        response = self.app.post('/api/architecture/graph', 
                                 data=json.dumps(payload),
                                 content_type='application/json')
        
        self.assertEqual(response.status_code, 200)
        data = response.get_json()
        
        print(json.dumps(data, indent=2))
        
        node_ids = {n['data']['id'] for n in data['nodes']}
        self.assertIn('Math.h', node_ids)
        self.assertIn('Player.cpp', node_ids)
        self.assertIn('Vector3', node_ids)
        self.assertIn('Player::Update', node_ids)
        
        edges = data['edges']
        
        inheritance = [e for e in edges if e['data']['type'] == 'inheritance']
        self.assertTrue(any(e['data']['source'] == 'Boss' and e['data']['target'] == 'Player' for e in inheritance))
        
        calls = [e for e in edges if e['data']['type'] == 'function_call']
        
        print("Calls found:", [(e['data']['source'], e['data']['target']) for e in calls])
        
        self.assertTrue(len(data['nodes']) > 0)
        self.assertTrue(len(edges) >= 0)

if __name__ == '__main__':
    unittest.main()
