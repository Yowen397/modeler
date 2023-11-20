// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.7.0 <0.9.0;
contract Storage {
    uint256 number;
    function store(uint256 num) public {
        uint256 test_v;
        number = num;
        test_v = num + 1;
        require(number==num);
    }
    function retrieve(uint256 p1, uint256 p2) public view returns (uint256){
        p1 + p2;
        assert(p1<=5);
        return number;
    }
}