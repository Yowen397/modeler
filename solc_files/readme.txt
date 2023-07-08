编译命令如下：
solc -o ./output/  --combined-json ast  storage.solc --overwrite
通过solc工具生成AST，以json格式存在

另：
solc版本可以通过solc-select工具管理
solc-select工具安装方式为：
pip3 install solc-select
该工具具体使用方法可以查看其说明