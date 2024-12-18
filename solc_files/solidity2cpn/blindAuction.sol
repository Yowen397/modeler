contract BlindAuction {
    struct Bid {
        bytes32 blindedBid;
        uint deposit;
    }
    mapping(address => Bid[]) bids;
    mapping(address => uint) pendingReturns;
    address highestBidder;
    uint highestBid;

    function bid(bytes32 blindedBid) public payable {
        bids[msg.sender].push(Bid(blindedBid, msg.value));
        pendingReturns[msg.sender] += msg.value;
    }

    function reveal(uint[] calldata values, bytes32[] calldata secrets) public {
        require(values.length == secrets.length);
        for (
            uint i = 0;
            i < values.length && i < bids[msg.sender].length;
            i++
        ) {
            Bid bid1 = bids[msg.sender][i];
            uint value = values[i];
            bytes32 secret = secrets[i];
            if (
                bid1.blindedBid == keccak256(abi.encodePacked(value, secret)) &&
                bid1.deposit >= value &&
                value > highestBid
            ) {
                highestBid = value;
                highestBidder = msg.sender;
            }
        }
    }

    function withdraw() public {
        uint amount = pendingReturns[msg.sender];
        if (amount > 0) {
            if (msg.sender != highestBidder) msg.sender.transfer(amount);
            else msg.sender.transfer(amount - highestBid);

            pendingReturns[msg.sender] = 0;
        }
    }
}
