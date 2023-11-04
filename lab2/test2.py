import pathlib
import re
import subprocess
import unittest


class TestLab2(unittest.TestCase):

    def _make():
        result = subprocess.run(['make'], capture_output=True, text=True)
        return result

    def _make_clean():
        result = subprocess.run(['make', 'clean'],
                                capture_output=True, text=True)
        return result

    @classmethod
    def setUpClass(cls):
        cls.make = cls._make().returncode == 0

    @classmethod
    def tearDownClass(cls):
        cls._make_clean()

    #-------------------------------------------------------------  
    def test_basic(self):
        #redefine these for each test
        processes_str = """4
1, 10, 70
2, 20, 40
3, 40, 10
4, 50, 40
        """
        expected_wait = 70.00
        expected_response = 12.25
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt', '10'), capture_output=True)
        expected_str = f"Average wait time: {'{:.2f}'.format(expected_wait)}\nAverage response time: {'{:.2f}'.format(expected_response)}\n"

        self.assertEqual(rr_result.stdout.decode('utf-8'),expected_str)

    def test_shuffle(self):
        #redefine these for each test
        processes_str = """4
2, 20, 40
1, 10, 70
4, 50, 40
3, 40, 10
        """
        expected_wait = 70.00
        expected_response = 12.25
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt', '10'), capture_output=True)
        expected_str = f"Average wait time: {'{:.2f}'.format(expected_wait)}\nAverage response time: {'{:.2f}'.format(expected_response)}\n"

        self.assertEqual(rr_result.stdout.decode('utf-8'),expected_str)
    
    def test_easy(self):
        #redefine these for each test
        processes_str = """4
1, 10, 10
2, 20, 10
3, 30, 10
4, 40, 10
        """
        expected_wait = 1.5
        expected_response = 1.5
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt', '10'), capture_output=True)
        expected_str = f"Average wait time: {'{:.2f}'.format(expected_wait)}\nAverage response time: {'{:.2f}'.format(expected_response)}\n"

        self.assertEqual(rr_result.stdout.decode('utf-8'),expected_str)

    def test_arrive_during_context_switch(self):
        #redefine these for each test
        processes_str = """4
1, 10, 20
2, 20, 30
3, 30, 20
4, 10, 10
        """
        expected_wait = 26.25
        expected_response = 14.25
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt', '10'), capture_output=True)
        expected_str = f"Average wait time: {'{:.2f}'.format(expected_wait)}\nAverage response time: {'{:.2f}'.format(expected_response)}\n"

        self.assertEqual(rr_result.stdout.decode('utf-8'),expected_str)

    def test_large_n(self):
        #redefine these for each test
        processes_str = """1000
        """
        for i in range (1000):
            processes_str += f"{i}, 1, 1\n"

        expected_wait = 999.0
        expected_response = 999.0
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt', '10'), capture_output=True)
        expected_str = f"Average wait time: {'{:.2f}'.format(expected_wait)}\nAverage response time: {'{:.2f}'.format(expected_response)}\n"

        self.assertEqual(rr_result.stdout.decode('utf-8'),expected_str)

    def test_median(self):
        #redefine these for each test
        processes_str = """4
1, 10, 70
2, 20, 40
3, 40, 10
4, 50, 40
        """
        expected_wait = 81.75
        expected_response = 13.75
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt', 'median'), capture_output=True)
        expected_str = f"Average wait time: {'{:.2f}'.format(expected_wait)}\nAverage response time: {'{:.2f}'.format(expected_response)}\n"
    #-------------------------------------------------------------
    def test_empty(self):
        #redefine these for each test
        processes_str = """0
1, 10, 70
        """
        expected_wait = 100.0
        expected_response = 53.00
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt', '3'), capture_output=True)
        expected_str = f"no processes\n"
        
        self.assertEqual(rr_result.stderr.decode('utf-8'),expected_str)

    def test_incomplete(self):
        #redefine these for each test
        processes_str = """3
1, 10, 70
        """
        expected_wait = 100.0
        expected_response = 53.00
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt', '3'), capture_output=True)
        expected_str = f"missing integer\n"
        
        self.assertEqual(rr_result.stderr.decode('utf-8'),expected_str)

    def test_no_quant(self):
        #redefine these for each test
        processes_str = """3
1, 10, 70
        """
        expected_wait = 100.0
        expected_response = 53.00
        #end of redfinition section

        file_path = "processes.txt"
        with open(file_path, 'w') as file:
            content = processes_str
            file.write(content)

        self.assertTrue(self.make, msg='make failed')
        rr_result = subprocess.run(('./rr', 'processes.txt'), capture_output=True)
        expected_str = f"./rr: usage: ./rr file quantum\n"
        
        self.assertEqual(rr_result.stderr.decode('utf-8'),expected_str)

if __name__ == "__main__":
    unittest.main()