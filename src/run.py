'''
批量运行测试
'''
import glob
import os

# 测试样例目录
tests_dir = "../solc_files/tests"
# 语法树输出目录
output_dir = "../solc_files/output2"

def get_all_files(directory):
    file_list = glob.glob(directory+"/*.sol", recursive=True)
    return file_list

if __name__ == "__main__":
    file_list = get_all_files(tests_dir)
    # 运行命令
    for f in file_list:
        cmd = "./modeler "
        basename = os.path.basename(f)
        basename_without_ext, basename_ext = os.path.splitext(basename)
        
        cmd += output_dir+"/"+basename_without_ext+"/combined.json "
        
        cmd += " 1>> record.txt 2>> error.txt"
        print(cmd)
        os.system(cmd)
    