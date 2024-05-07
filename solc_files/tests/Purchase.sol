// SPDX-License-Identifier: GPL-3.0
pragma solidity ^0.8.4;
contract Purchase {
    uint public value;
    address payable public seller;
    address payable public buyer;

    // State: Created=0,Locked=1,Release=2,Inactive=3
    uint public state = 0;

    // 确保 `msg.value` 是一个偶数。
    // 如果是奇数，除法会截断。
    // 通过乘法检查它不是一个奇数。
    // constructor() payable {
    //     seller = payable(msg.sender);
    //     value = msg.value / 2;
    //     require((2 * value) == msg.value);
    // }

    /// 终止购买并收回 ether。
    /// 只能由卖方在合同锁定前能调用。
    function abort() external
    {
        require(msg.sender == seller);
        require(state == 0);

        state = 3;
        // 我们在这里直接使用 `transfer`。
        // 它可以安全地重入。
        // 因为它是这个函数中的最后一次调用，
        // 而且我们已经改变了状态。
        seller.transfer(address(this).balance);
    }

    /// 买方确认购买。
    /// 交易必须包括 `2 * value` ether。
    /// Ether 将被锁住，直到调用 confirmReceived。
    function confirmPurchase() external payable
    {
        require(state == 0);
        uint256 V;
        V = 2 * value;
        require(msg.value == V);

        buyer = payable(msg.sender);
        state = 1;
    }

    /// 确认您（买方）已经收到了该物品。
    /// 这将释放锁定的 ether。
    function confirmReceived() external
    {
        require(msg.sender == buyer);
        require(state == 1);
        
        // 首先改变状态是很重要的，否则的话，
        // 下面使用 `send` 调用的合约可以在这里再次调用。
        state = 2;

        buyer.transfer(value);
    }

    /// 该功能为卖家退款，
    /// 即退还卖家锁定的资金。
    function refundSeller() external
    {
        require(msg.sender == seller);
        require(state == 2);

        // 首先改变状态是很重要的，否则的话，
        // 下面使用 `send` 调用的合约可以在这里再次调用。
        state = 3;

        uint256 V ;
        V = 3 * value;
        seller.transfer(V);
    }
}