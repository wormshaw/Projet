// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

contract PackageStorage {
    struct ClusterHead {
        string MAC_address;
        int round_number_as_CH;
    }

    struct Node {
        string MAC_address;
        int[] RSSIs;
        string[] times;
    }

    mapping(string => ClusterHead) private clusterheads; // Stocke les informations des clusterheads
    mapping(string => Node[]) private nodes; // Stocke les informations des nœuds membres par clusterhead
    string[] private clusterHeadAddresses; // Stocke les adresses MAC des clusterheads

    // Fonction pour stocker les informations d'un clusterhead
    function addClusterHead(string memory macAddress, int roundNumber) public {
        clusterheads[macAddress] = ClusterHead(macAddress, roundNumber);
        clusterHeadAddresses.push(macAddress);
    }

    // Fonction pour stocker les informations des nœuds membres et les associer à un clusterhead existant
    function addNodes(string[] memory macAddresses, int[] memory RSSIs, string[] memory times, string memory clusterheadMACAddress) public {
        require(macAddresses.length == RSSIs.length && macAddresses.length == times.length, "Invalid input arrays.");
        
        require(clusterheads[clusterheadMACAddress].round_number_as_CH != 0, "Clusterhead does not exist.");
        
        Node[] storage nodeArr = nodes[clusterheadMACAddress];
        
        for (uint i = 0; i < macAddresses.length; i++) {
            nodeArr.push(Node(macAddresses[i], new int[](0), new string[](0)));
            nodeArr[nodeArr.length - 1].RSSIs.push(RSSIs[i]);
            nodeArr[nodeArr.length - 1].times.push(times[i]);
        }
    }

    // Fonction pour récupérer les informations d'un clusterhead à partir de la MAC_address
    function getClusterHead(string memory macAddress) public view returns (string memory, int) {
        ClusterHead storage clusterhead = clusterheads[macAddress];
        return (clusterhead.MAC_address, clusterhead.round_number_as_CH);
    }

    // Fonction pour récupérer un tableau de RSSI et de time d'un noeud à partir de sa MAC_address
    function getNodeInfo(string memory nodeMACAddress) public view returns (int[] memory, string[] memory) {
        uint count = 0;
        for (uint i = 0; i < clusterHeadAddresses.length; i++) {
            Node[] storage nodeArr = nodes[clusterHeadAddresses[i]];
            for (uint j = 0; j < nodeArr.length; j++) {
                if (keccak256(abi.encodePacked(nodeArr[j].MAC_address)) == keccak256(abi.encodePacked(nodeMACAddress))) {
                    count++;
                }
            }
        }

        int[] memory rssiValues = new int[](count);
        string[] memory timeValues = new string[](count);

        uint index = 0;
        for (uint i = 0; i < clusterHeadAddresses.length; i++) {
            Node[] storage nodeArr = nodes[clusterHeadAddresses[i]];
            for (uint j = 0; j < nodeArr.length; j++) {
                if (keccak256(abi.encodePacked(nodeArr[j].MAC_address)) == keccak256(abi.encodePacked(nodeMACAddress))) {
                    rssiValues[index] = nodeArr[j].RSSIs[nodeArr[j].RSSIs.length - 1];
                    timeValues[index] = nodeArr[j].times[nodeArr[j].times.length - 1];
                    index++;
                }
            }
        }

        return (rssiValues, timeValues);
    }
}
