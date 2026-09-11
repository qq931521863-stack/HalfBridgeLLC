"""Do not report an interrupted or partial RPC simulation as passing."""
import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from run_plecs import validate_result

class CompletionTests(unittest.TestCase):
    def test_partial_result_rejected(self):
        with self.assertRaisesRegex(AssertionError,'Incomplete simulation'):
            validate_result({'Time':[0,1.095],'Values':[[0,0]]},2)
    def test_complete_result_transposed(self):
        self.assertEqual(validate_result({'Time':[0,2],'Values':[[1,2],[3,4],[5,6]]},2),
                         ([0,2],[[1,3,5],[2,4,6]]))
    def test_empty_result_rejected(self):
        with self.assertRaises(AssertionError):validate_result({'Time':[],'Values':[]},2)
    def test_inconsistent_signal_length_rejected(self):
        with self.assertRaises(AssertionError):
            validate_result({'Time':[0,2],'Values':[[0,1],[0]]},2)

if __name__=='__main__':unittest.main()
