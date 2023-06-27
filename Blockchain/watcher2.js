const bodyParser = require('body-parser');
const fs = require('fs');
const Web3 = require('web3');
const  Wallet = require('ethereumjs-wallet').default
const Tx = require('ethereumjs-tx').Transaction;


const web3 = new Web3('https://rpc.testnet.fantom.network/');

const chokidar = require('chokidar');



const contractABI = require('./contractABI2.json');
const { type } = require('os');


const contractAddress = '0x4584C9c2547e5290c1e2333dC5A7907c0125b4cb';
const contractInstance = new web3.eth.Contract(contractABI, contractAddress);


const privateKey = 'ae44534475491acd6f5b5473e8cfcec9d62d14b9539be51716f15febcf6407aa';
const wallet = Wallet.fromPrivateKey(Buffer.from(privateKey, 'hex'));


const folderToWatch = '/home/elias/repos/ns-3-allinone/ns-3-dev/Packages.txt';
const previousFileStates = new Map();


  
  
const filePath = folderToWatch;
path = filePath;

function watchFile() {
  let currentLines = [];


  const watcher = chokidar.watch(filePath, {
    awaitWriteFinish: {
      stabilityThreshold: 5000,
      pollInterval: 100
    }
  });

  watcher.on('change', async (path) => {
    console.log(`Le fichier ${path} a été modifié.`);

    const newLines = await readNewLinesFromFile(filePath, currentLines);
    console.log(`Ajout de ${newLines.length} nouvelle(s) ligne(s) dans le fichier.`);
    parseLines(newLines);

    currentLines = await readAllLinesFromFile(filePath);
  });

  process.on('SIGINT', () => {
    watcher.close();
    process.exit(0);
  });
}

async function readNewLinesFromFile(filePath, currentLines) {
  const fileContent = await fs.promises.readFile(filePath, 'utf8');
  const lines = fileContent.split('\n');

  const newLines = lines.filter(line => !currentLines.includes(line));
  return newLines;
}

async function readAllLinesFromFile(filePath) {
  const fileContent = await fs.promises.readFile(filePath, 'utf8');
  const lines = fileContent.split('\n');
  return lines;
}



watchFile();





 

// Fonction pour analyser les lignes du fichier texte
async function parseLines(lines) {
    let roundData = {}; // Stocke les informations du round par clusterhead
  
    for (let i = 0; i < lines.length; i++) {
      let line = lines[i].trim();
      if (line !== '') {
        let [round, clusterheadMAC, nodeMAC, RSSI, time] = line.split(' ');
  
        if (!roundData[round]) {
          roundData[round] = {};
        }
  
        if (!roundData[round][clusterheadMAC]) {
          roundData[round][clusterheadMAC] = [];
        }
  
        roundData[round][clusterheadMAC].push({
          nodeMAC: nodeMAC,
          RSSI: parseInt(RSSI),
          time: time
        });
      }
    }
  
    for (let round in roundData) {
      console.log('Round', round);
  
      for (let clusterheadMAC in roundData[round]) {
        let nodeData = roundData[round][clusterheadMAC];
        console.log('Clusterhead', clusterheadMAC);
  
        // Appeler la fonction addCH
        addCH(clusterheadMAC, parseInt(round));
        await new Promise(resolve => setTimeout(resolve, 5000));
  
        let macAddresses = [];
        let RSSIs = [];
        let times = [];
  
        for (let i = 0; i < nodeData.length; i++) {
          macAddresses.push(nodeData[i].nodeMAC);
          RSSIs.push(nodeData[i].RSSI);
          times.push(nodeData[i].time);
        }
  
        // Appeler la fonction addNodes
        addNodes(macAddresses, RSSIs, times, clusterheadMAC);
  
        await new Promise(resolve => setTimeout(resolve, 5000)); // Attendre 3 secondes
      }
  
      console.log('-------------------');
    }
  }
  




  async function addCH(MAC_address, round) {
    const account = web3.eth.accounts.privateKeyToAccount(privateKey);
    const gasPrice = await web3.eth.getGasPrice();
    const nonce = await web3.eth.getTransactionCount(account.address);
  
    
    const data = contractInstance.methods.addClusterHead(MAC_address,round).encodeABI();
    const txObject = {
      nonce: nonce,
      gasPrice: gasPrice,
      gasLimit: web3.utils.toHex(300000),
      to: contractAddress,
      data: data,
    };
    const signedTx = await web3.eth.accounts.signTransaction(txObject, privateKey);
    const receipt = await web3.eth.sendSignedTransaction(signedTx.rawTransaction);
  
    console.log('CH transaction processed successfully.');
    console.log('Transaction Hash:', receipt.transactionHash);
    return new Promise((resolve) => {
      setTimeout(() => {
      }, 200);
    });
  
  }



  async function addNodes(macAddresses, RSSIs, times, clusterheadMAC) {
    const account = web3.eth.accounts.privateKeyToAccount(privateKey);
    const gasPrice = await web3.eth.getGasPrice();
    const nonce = await web3.eth.getTransactionCount(account.address);

  
    const data = contractInstance.methods.addNodes(macAddresses, RSSIs, times, clusterheadMAC).encodeABI();
    const txObject = {
      nonce: nonce,
      gasPrice: gasPrice,
      gasLimit: web3.utils.toHex(8000000),
      to: contractAddress,
      data: data,
    };
    const signedTx = await web3.eth.accounts.signTransaction(txObject, privateKey);
    const receipt = await web3.eth.sendSignedTransaction(signedTx.rawTransaction);
  
    console.log('Nodes transaction processed successfully.');
    console.log('Transaction Hash:', receipt.transactionHash);
  }


