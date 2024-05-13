// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.7.0 <0.9.0;
// 这将报告一个由于废弃的 selfdestruct 而产生的警告
contract SimplePaymentChannel {
    address payable public sender;      // 发送付款的账户。
    address payable public recipient;   // 接收付款的账户。
    uint256 public expiration;  // 超时时间，以防接收者永不关闭支付通道。

    constructor (address payable recipientAddress, uint256 duration)
        payable
    {
        sender = payable(msg.sender);
        recipient = recipientAddress;
        expiration = block.timestamp + duration;
    }

    /// 接收者可以在任何时候通过提供发送者签名的金额来关闭通道，
    /// 接收者将获得该金额，其余部分将返回发送者。
    function close(uint256 amount, bytes memory signature) external {
        require(msg.sender == recipient);
        require(isValidSignature(amount, signature));

        recipient.transfer(amount);
        selfdestruct(sender);
    }

    /// 发送者可以在任何时候延长到期时间。
    function extend(uint256 newExpiration) external {
        require(msg.sender == sender);
        require(newExpiration > expiration);

        expiration = newExpiration;
    }

    /// 如果达到超时时间而接收者没有关闭通道，
    /// 那么以太就会被释放回给发送者。
    function claimTimeout() external {
        require(block.timestamp >= expiration);
        selfdestruct(sender);
    }

    function isValidSignature(uint256 amount, bytes memory signature)
        internal
        view
        returns (bool)
    {
        bytes32 message = prefixed(keccak256(abi.encodePacked(this, amount)));

        // 检查签名是否来自付款方。
        return recoverSigner(message, signature) == sender;
    }

    /// 下面的所有功能是取自 '创建和验证签名' 的章节。

    function splitSignature(bytes memory sig)
        internal
        pure
        returns (uint8 v, bytes32 r, bytes32 s)
    {
        require(sig.length == 65);

        assembly {
            // 前32个字节，在长度前缀之后。
            r := mload(add(sig, 32))
            // 第二个32字节。
            s := mload(add(sig, 64))
            // 最后一个字节（下一个32字节的第一个字节）。
            v := byte(0, mload(add(sig, 96)))
        }

        return (v, r, s);
    }

    function recoverSigner(bytes32 message, bytes memory sig)
        internal
        pure
        returns (address)
    {
        (uint8 v, bytes32 r, bytes32 s) = splitSignature(sig);

        return ecrecover(message, v, r, s);
    }

    /// 构建一个前缀哈希值，以模仿eth_sign的行为。
    function prefixed(bytes32 hash) internal pure returns (bytes32) {
        return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
    }
}