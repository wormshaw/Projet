// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;


contract NodeStorage {
    struct ClusterHead {
        string MAC_address;
        int energy;
        int round_number_as_CH;
    }

    struct Node {
        string MAC_address;
        int energy;
        int rounds_since_CH;
        string clusterheadMAC_address; // MAC_address du clusterhead associé
    }

    mapping(string => ClusterHead) private clusterheads; // Stocke les informations des clusterheads
    mapping(string => Node) private nodes; // Stocke les informations des noeuds membres
    string[] private clusterHeadAddresses; // Stocke les adresses MAC des clusterheads
    string[] private nodeAddresses; // Stocke les adresses MAC des noeuds membres

    // Fonction pour stocker les informations d'un clusterhead
    function addClusterHead(string memory macAddress, int energy, int roundNumber) public {
        clusterheads[macAddress] = ClusterHead(macAddress, energy, roundNumber);
        clusterHeadAddresses.push(macAddress);
    }

    // Fonction pour stocker les informations des nœuds membres et les associer à un clusterhead existant
    function addNodes(string[] memory macAddresses, int[] memory energies, int[] memory roundsSinceCH, string memory clusterheadMACAddress) public {
        require(clusterheads[clusterheadMACAddress].round_number_as_CH > 0, "Clusterhead does not exist.");
        require(macAddresses.length == energies.length && macAddresses.length == roundsSinceCH.length, "Invalid input arrays.");
        
        for (uint i = 0; i < macAddresses.length; i++) {
            nodes[macAddresses[i]] = Node(macAddresses[i], energies[i], roundsSinceCH[i], clusterheadMACAddress);
            nodeAddresses.push(macAddresses[i]);
        }
    }

    // Fonction pour récupérer les informations d'un clusterhead à partir de la MAC_address
    function getClusterHead(string memory macAddress) public view returns (string memory, int, int) {
        ClusterHead storage clusterhead = clusterheads[macAddress];
        return (clusterhead.MAC_address, clusterhead.energy, clusterhead.round_number_as_CH);
    }

    // Fonction pour récupérer les informations des nœuds membres d'un cluster à partir de la MAC_address du clusterhead
    function getNodesFromClusterHead(string memory macAddress) public view returns (string[] memory, int[] memory, int[] memory) {
        uint nodeCount = 0;
        for (uint i = 0; i < nodeAddresses.length; i++) {
            if (keccak256(bytes(nodes[nodeAddresses[i]].clusterheadMAC_address)) == keccak256(bytes(macAddress))) {
                nodeCount++;
            }
        }

        string[] memory nodeAddressesResult = new string[](nodeCount);
        int[] memory nodeEnergies = new int[](nodeCount);
        int[] memory nodeRoundsSinceCH = new int[](nodeCount);

        uint currentIndex = 0;
        for (uint i = 0; i < nodeAddresses.length; i++) {
            if (keccak256(bytes(nodes[nodeAddresses[i]].clusterheadMAC_address)) == keccak256(bytes(macAddress))) {
                nodeAddressesResult[currentIndex] = nodes[nodeAddresses[i]].MAC_address;
                nodeEnergies[currentIndex] = nodes[nodeAddresses[i]].energy;
                nodeRoundsSinceCH[currentIndex] = nodes[nodeAddresses[i]].rounds_since_CH;
                currentIndex++;
            }
        }

        return (nodeAddressesResult, nodeEnergies, nodeRoundsSinceCH);
    }

    // Fonction pour récupérer les informations des clusterheads à partir du numéro de round
    function getClusterHeadsFromRound(int roundNumber) public view returns (string[] memory, int[] memory, int[] memory) {
        uint clusterHeadCount = 0;
        for (uint i = 0; i < clusterHeadAddresses.length; i++) {
            if (clusterheads[clusterHeadAddresses[i]].round_number_as_CH == roundNumber) {
                clusterHeadCount++;
            }
        }

        string[] memory clusterHeadAddressesResult = new string[](clusterHeadCount);
        int[] memory clusterHeadEnergies = new int[](clusterHeadCount);
        int[] memory clusterHeadRoundNumbers = new int[](clusterHeadCount);

        uint currentIndex = 0;
        for (uint i = 0; i < clusterHeadAddresses.length; i++) {
            if (clusterheads[clusterHeadAddresses[i]].round_number_as_CH == roundNumber) {
                clusterHeadAddressesResult[currentIndex] = clusterheads[clusterHeadAddresses[i]].MAC_address;
                clusterHeadEnergies[currentIndex] = clusterheads[clusterHeadAddresses[i]].energy;
                clusterHeadRoundNumbers[currentIndex] = clusterheads[clusterHeadAddresses[i]].round_number_as_CH;
                currentIndex++;
            }
        }

        return (clusterHeadAddressesResult, clusterHeadEnergies, clusterHeadRoundNumbers);
    }
}
