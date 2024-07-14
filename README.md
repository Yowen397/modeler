# modeler
建模智能合约并生成状态空间进行验证



### ~~切换Tokens的方法~~ ###
~~直接注释/取消注释StateSpace.h文件中的宏定义 \
#define USE_TOKENS~~

### 切换是否应用合并状态压缩空间 ###
直接注释/取消注释StateSpace.h文件中的宏定义 \
#define MERGE_STATUS

### 运行 ###
solc_files目录下scripts.py运行可批量生成语法树，对应测试集文件在tests目录下，生成的语法树文件以path/SmartContractName/combined.json格式存在 \
批量运行：将src目录下的run.py复制到bin目录（cmake生成可执行文件目录）下，执行脚本