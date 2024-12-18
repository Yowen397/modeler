pragma solidity ^0.8.0;

contract BlindAuction {
    struct Bid {
        bytes32 blindedBid;
        uint deposit;
    }

    mapping(address => Bid[]) public bids;
    mapping(address => uint) public pendingReturns;
    address public highestBidder;
    uint public highestBid;

    event BidPlaced(address indexed bidder, bytes32 blindedBid, uint amount);
    event Reveal(address indexed bidder, uint value, bytes32 secret);
    event Withdraw(address indexed bidder, uint amount);
    event NewHighestBid(address indexed bidder, uint bidAmount);

    function bid(bytes32 blindedBid) public payable {
        require(msg.value > 0, "Bid must be greater than 0");
        bids[msg.sender].push(Bid(blindedBid, msg.value));
        pendingReturns[msg.sender] += msg.value;
        emit BidPlaced(msg.sender, blindedBid, msg.value);
    }

    function reveal(uint[] calldata values, bytes32[] calldata secrets) public {
        require(values.length == secrets.length, "Mismatched input lengths");
        for (uint i = 0; i < values.length && i < bids[msg.sender].length; i++) {
            Bid storage bid1 = bids[msg.sender][i];
            uint value = values[i];
            bytes32 secret = secrets[i];
            if (bid1.blindedBid == keccak256(abi.encodePacked(value, secret)) && bid1.deposit >= value) {
                if (value > highestBid) {
                    highestBid = value;
                    highestBidder = msg.sender;
                    emit NewHighestBid(msg.sender, value);
                }
                pendingReturns[msg.sender] -= value;
                emit Reveal(msg.sender, value, secret);
            }
        }
    }

    function withdraw() public {
        uint amount = pendingReturns[msg.sender];
        if (amount > 0) {
            uint refundAmount = amount;
            if (msg.sender == highestBidder) {
                refundAmount -= highestBid;
            }
            pendingReturns[msg.sender] = 0;
            payable(msg.sender).transfer(refundAmount);
            emit Withdraw(msg.sender, refundAmount);
        }
    }

    function getBidCount(address bidder) public view returns (uint) {
        return bids[bidder].length;
    }

    function getHighestBid() public view returns (address, uint) {
        return (highestBidder, highestBid);
    }
}
