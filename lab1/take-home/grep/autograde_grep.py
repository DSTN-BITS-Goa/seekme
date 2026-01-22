import json
import unittest

scores = {"scores": {}}


class TestGrepClass(unittest.TestCase):

    def get_fail_message(self, expected, actual):
        return "Expected: " + str(expected) + "\nActual: " + str(actual)

    def update_scores(self, test_case, score, expr1, expr2):
        if expr1 == expr2:
            scores["scores"][test_case] = score
        else:
            scores["scores"][test_case] = 0

    def test_grep_basic(self):
        # Manual test for grep_search_file() - 2 marks
        # Creates a test file for manual verification
        import os
        
        test_file = "grep_test_file.txt"
        test_content = """hello world
this is a test
Hello World
another line
test pattern here
no match line
"""
        
        try:
            with open(test_file, 'w') as f:
                f.write(test_content)
            
            # Set to 0 - requires manual verification for marks
            scores["scores"]["grep_basic"] = 0
            
            print("\n=== Grep Manual Test (2 marks) ===")
            print(f"Test file created: {test_file}")
            print("\nManual verification checklist:")
            print("1. grep_search_file() reads file and finds matches")
            print("2. Pattern matching works (use grep_match_pattern)")
            print("3. -n flag adds line numbers correctly")
            print("4. -v flag inverts matches correctly")
            print("\nTest file content:")
            print(test_content)
            print("Expected: Pattern 'test' should match lines 2, 5")
            print("Expected: Pattern 'hello' (case-insensitive) should match lines 1, 3")
            
        except Exception as e:
            scores["scores"]["grep_basic"] = 0
            print(f"Error creating test file: {e}")

# If this script is executed directly, run the tests
if __name__ == "__main__":
    unittest.main(exit=False)
    
    # Add grep test if not present
    if "grep_basic" not in scores["scores"]:
        scores["scores"]["grep_basic"] = 0

    print(json.dumps({"_presentation": "semantic"}, separators=(',', ': ')))
    print(json.dumps(scores, separators=(',', ': ')))