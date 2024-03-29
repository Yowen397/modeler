// SPDX-License-Identifier: GPL-3.0

pragma solidity >=0.8.2 <0.9.0;

contract SimpleDAO {
    mapping (address => uint) public credit;

    function donate(address _to, uint _value) public payable{
        credit[_to] += _value;
    }

    function withdraw(uint _amount) public{
        if(credit[msg.sender] >= _amount){
            msg.sender.call{value:_amount}("");
            credit[msg.sender] -= _amount;
        }
    }

    function queryCredit(address _account) public view returns(uint){
        return credit[_account];
    }

}
