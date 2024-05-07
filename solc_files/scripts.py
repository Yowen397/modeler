import glob
import os

# 测试样例目录
tests_dir = "./tests"
# 语法树输出目录
output_dir = "./output2"

def get_all_files(directory):
    file_list = glob.glob(directory+"/*.sol", recursive=True)
    return file_list

def get_ast(files: list, output_dir: str):

    for file in files:
        cmd = "solc --combined-json ast "
        basename = os.path.basename(file)
        basename_without_ext, basename_ext = os.path.splitext(basename)
        cmd += " -o " + output_dir+"/"+ basename_without_ext
        
        cmd += " "+file
        cmd+= " --overwrite "
        print(cmd)
        os.system(cmd)


if __name__ == "__main__":

    # 获取对应目录下所有的.sol文件
    file_list = get_all_files(tests_dir)

    # 调用solc编译生成语法树文件
    get_ast(file_list, output_dir)
    
    print("Generated AST of files in ["+tests_dir+"] to ["+output_dir+"]")