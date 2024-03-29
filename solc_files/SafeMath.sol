// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.7.0 <0.9.0;
contract SafeMath {
    uint256 result;
    function add(uint256 a, uint256 b) public {
        result = a+b;
        require(result>a);
        require(result>b);
    }
    function sub(uint256 a, uint256 b) public {
        require(a>b);
        result = a-b;
    }
}