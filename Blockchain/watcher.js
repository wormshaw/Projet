const bodyParser = require('body-parser');
const fs = require('fs');
const Web3 = require('web3');
const  Wallet = require('ethereumjs-wallet').default
const Tx = require('ethereumjs-tx').Transaction;


const web3 = new Web3('https://fantom-testnet.public.blastapi.io');

const chokidar = require('chokidar');



const contractABI = require('./contractABI.json');
const { type } = require('os');


const contractAddress = '0x81174A94de07c3f1410A44e610a770308f658B2F';
const contractInstance = new web3.eth.Contract(contractABI, contractAddress);


const privateKey = 'ae44534475491acd6f5b5473e8cfcec9d62d14b9539be51716f15febcf6407aa';
const wallet = Wallet.fromPrivateKey(Buffer.from(privateKey, 'hex'));


const folderToWatch = 'C:/Users/badem/OneDrive/Bureau/watcher/leach_data_ex.txt';
const previousFileStates = new Map();





async function processFileLine1(line1) {
  const account = web3.eth.accounts.privateKeyToAccount(privateKey);
  const gasPrice = await web3.eth.getGasPrice();
  const nonce = await web3.eth.getTransactionCount(account.address);

  // transaction addClusterHead
  line1 = line1.slice(3);
  const [macAddress, energyStr, roundNumber] = line1.trim().split(' ');
  const energy = parseInt(energyStr); // Convertir en valeur entière

  const data = contractInstance.methods.addClusterHead(macAddress, energy, parseInt(roundNumber)).encodeABI();
  const txObject = {
    nonce: nonce,
    gasPrice: gasPrice,
    gasLimit: web3.utils.toHex(300000),
    to: contractAddress,
    data: data,
  };
  const signedTx = await web3.eth.accounts.signTransaction(txObject, privateKey);
  const receipt = await web3.eth.sendSignedTransaction(signedTx.rawTransaction);

  console.log('Line 1 processed successfully.');
  console.log('Transaction Hash:', receipt.transactionHash);
  return new Promise((resolve) => {
    setTimeout(() => {
      resolve(macAddress);
    }, 200);
  });

}

async function processFileLine2(line2,macAddress) {
  //await processFileLine1(); 
  const account = web3.eth.accounts.privateKeyToAccount(privateKey);
  const gasPrice = await web3.eth.getGasPrice();
  const nonce = await web3.eth.getTransactionCount(account.address);

  // transaction addNodes
  const nodeDataArray = line2.trim().split(';');
  nodeDataArray.pop();
  const macAddresses = [];
  const energies = [];
  const roundsSinceCH = [];

  for (let i = 0; i < nodeDataArray.length; i++) {
    const [nodeMacAddress, nodeEnergyStr, nodeRoundsSinceCH] = nodeDataArray[i].trim().split(' ');
    const nodeEnergy = parseInt(nodeEnergyStr);
    macAddresses.push(nodeMacAddress);
    energies.push(nodeEnergy);
    roundsSinceCH.push(parseInt(nodeRoundsSinceCH));
  }


  const data = contractInstance.methods.addNodes(macAddresses, energies, roundsSinceCH, macAddress).encodeABI();
  const txObject = {
    nonce: nonce,
    gasPrice: gasPrice,
    gasLimit: web3.utils.toHex(3000000),
    to: contractAddress,
    data: data,
  };
  const signedTx = await web3.eth.accounts.signTransaction(txObject, privateKey);
  const receipt = await web3.eth.sendSignedTransaction(signedTx.rawTransaction);

  console.log('Line 2 processed successfully.');
  console.log('Transaction Hash:', receipt.transactionHash);
}



const watcher = chokidar.watch(folderToWatch, {
  persistent: true,
});

watcher.on('change', (path) => {
  fs.readFile(path, 'utf8', async (err, data) => {
    if (err) {
      console.error(`Erreur lors de la lecture du fichier ${path}:`, err);
      return;
    }

    const previousState = previousFileStates.get(path) || '';
    const currentState = data;

    // vérifier ajout de sauts de lignes
    if (currentState.length > previousState.length) {
      const addedNewlines = currentState.split('\n').length - previousState.split('\n').length;
      if (addedNewlines >= 2) { 
        console.log(`Ajout de ${addedNewlines} saut(s) de ligne dans le fichier ${path}.`);

        // récupérer les deux dernières lignes ajoutées
        const lines = currentState.split('\n');
        const startIndex = lines.length - addedNewlines - 1;
        const addedLines = lines.slice(startIndex, startIndex + addedNewlines);

        // appeler la fonction qui exécute les transactions
        for (let i = 0; i < addedLines.length; i += 2) {
          const line1 = addedLines[i];
          const line2 = addedLines[i + 1];

          try {
            const macAddress = await processFileLine1(line1);
            await processFileLine2(line2, macAddress);
          } catch (error) {
            console.error(error);
          }
        }
        
      }
    }

    previousFileStates.set(path, currentState);
  });
});

watcher.on('error', (error) => {
  console.error('Erreur lors de la surveillance des fichiers:', error);
});
