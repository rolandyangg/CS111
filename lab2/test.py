# import subprocess
# import random

# def generate_random():
#     res = ""
#     n = random.randint(1, 100)
#     res += str(n) + "\n"
#     for i in range(n):
#         res += str(i) + ", " +str(random.randint(1, 100)) + ", " + str(random.randint(1, 100)) + "\n"
#     return res

# def run_binary(binary_path):
#     try:
#         result = subprocess.check_output(binary_path, text=True, shell=True, stderr=subprocess.STDOUT)
#         return result
#     except subprocess.CalledProcessError as e:
#         print(f"Error running {binary_path}: {e.output}")
#         return None

# def check_output(input):
#     print("-----------------------------")
#     # yeah just replace this shit with the files to check yk
#     binary1 = run_binary("./rr processes.txt " + input)
#     binary2 = run_binary("./rr_aka processes.txt " + input)
#     binary3 = run_binary("./rr_shu processes.txt " + input)
#     binary4 = run_binary("./rr_mic processes.txt " + input)

#     if binary1 == binary2 == binary3:
#         print("Success: " + input)
#     else:
#         print("Akash: ")
#         print(binary2)
#         print("Roland: ")
#         print(binary1)
#         print("Shubham: ")
#         print(binary3)
#         print("Michael: ")
#         print(binary4)
#         return False
    
#     return True

# def main():
#     for x in range(100):
#         with open("processes.txt", "w") as file:
#             res = generate_random()
#             file.write(res)
        
#         # for z in range(5):
#         #     if (check_output(str(random.randint(1, 15))) == False):
#         #         print(res)
        
#         check_output("median")

# if __name__ == "__main__":
#     main()

import subprocess
import random

def generate_random():
    res = ""
    n = random.randint(1, 3)
    res += str(n) + "\n"
    for i in range(n):
        res += str(i) + ", " +str(random.randint(1, 10)) + ", " + str(random.randint(1, 10)) + "\n"
    return res

def run_binary(binary_path):
    try:
        result = subprocess.check_output(binary_path, text=True, shell=True, stderr=subprocess.STDOUT)
        return result
    except subprocess.CalledProcessError as e:
        print(f"Error running {binary_path}: {e.output}")
        return None

def check_output(input):
    # print("-----------------------------")
    # yeah just replace this shit with the files to check yk
    binary1 = run_binary("./rr processes.txt " + input)
    binary2 = run_binary("./rr_aka processes.txt " + input)

    if binary1 != binary2:
        print("Failure:")
        print("Roland: ")
        print(binary1)
        print("Akash: ")
        print(binary2)
        print(input)
        exit()

def main():
    for x in range(100):
        with open("processes.txt", "w") as file:
            file.write(generate_random())
        
        # for z in range(15):
        #     check_output(str(z + 1))
        
        check_output("median")

if __name__ == "__main__":
    main()