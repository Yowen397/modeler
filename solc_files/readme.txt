编译命令如下：
solc -o ./output/  --combined-json ast  storage.sol --overwrite
通过solc工具生成AST，以json格式存在

另：
solc版本可以通过solc-select工具管理
solc-select工具安装方式为：
pip3 install solc-select
该工具具体使用方法可以查看其说明


编译其它文件
solc -o ./output/storage/  --combined-json ast  storage.sol --overwrite
solc -o ./output/Purchase/ --combined-json ast Purchase.sol --overwrite
solc -o ./output/Ballot/ --combined-json ast Ballot.sol --overwrite
solc -o ./output/SimpleAution/ --combined-json ast SimpleAution.sol --overwrite
solc -o ./output/ReceiverPays/ --combined-json ast ReceiverPays.sol --overwrite
solc -o ./output/Timelock/ --combined-json ast Timelock.sol --overwrite