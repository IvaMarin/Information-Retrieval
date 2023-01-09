// we start with the TrieNode
const TrieNode = function (key) {
    // the "key" value will be the character in sequence 
    this.key = key;

    // we keep a reference to parent
    this.parent = null;

    // we have hash of children
    this.children = {};

    // check to see if the node is at the end
    this.end = false;

    this.getWord = function () {
        let output = [];
        let node = this;

        while (node !== null) {
            output.unshift(node.key);
            node = node.parent;
        }

        return output.join('');
    };
}

const Trie = function () {
    this.root = new TrieNode(null);
    // this.topK = 9;

    // inserts a word into the trie.
    this.insert = function (word) {
        let node = this.root; // we start at the root

        // for every character in the word
        for (let i = 0; i < word.length; i++) {
            // check to see if character node exists in children.
            if (!node.children[word[i]]) {
                // if it doesn't exist, we then create it.
                node.children[word[i]] = new TrieNode(word[i]);

                // we also assign the parent to the child node.
                node.children[word[i]].parent = node;
            }

            // proceed to the next depth in the trie.
            node = node.children[word[i]];

            // finally, we check to see if it's the last word.
            if (i == word.length - 1) {
                // if it is, we set the end flag to true.
                node.end = true;
            }
        }
    };

    // check if it contains a whole word.
    this.contains = function (word) {
        let node = this.root;

        // for every character in the word
        for (let i = 0; i < word.length; i++) {
            // check to see if character node exists in children.
            if (node.children[word[i]]) {
                // if it exists, proceed to the next depth of the trie.
                node = node.children[word[i]];
            } else {
                // doesn't exist, return false since it's not a valid word.
                return false;
            }
        }

        // we finished going through all the words, but is it a whole word?
        return node.end;
    };

    // returns every word with given prefix
    this.find = function (prefix) {
        let node = this.root;
        let output = [];

        // for every character in the prefix
        for (let i = 0; i < prefix.length; i++) {
            // make sure prefix actually has words
            if (node.children[prefix[i]]) {
                node = node.children[prefix[i]];
            } else {
                // there's none. just return it.
                return output;
            }
        }

        // recursively find all words in the node
        findAllWords(node, output);

        return output;
    };

    // recursive function to find all words in the given node.
    const findAllWords = (node, arr) => {
        // base case, if node is at a word, push to output
        if (node.end) {
            arr.unshift(node.getWord());
        }

        // iterate through each children, call recursive findAllWords
        for (let child in node.children) {
            findAllWords(node.children[child], arr);
            // if (arr.length > this.topK) {
            //     break;
            // }
        }
    }

    // removes a word from the trie.
    this.remove = function (word) {
        let root = this.root;

        if (!word) return;

        // recursively finds and removes a word
        const removeWord = (node, word) => {

            // check if current node contains the word
            if (node.end && node.getWord() === word) {

                // check and see if node has children
                let hasChildren = Object.keys(node.children).length > 0;

                // if has children we only want to un-flag the end node that marks the end of a word.
                // this way we do not remove words that contain/include supplied word
                if (hasChildren) {
                    node.end = false;
                } else {
                    // remove word by getting parent and setting children to empty dictionary
                    node.parent.children = {};
                }

                return true;
            }

            // recursively remove word from all children
            for (let key in node.children) {
                removeWord(node.children[key], word)
            }

            return false
        };

        // call remove word on root node
        removeWord(root, word);
    };
}


const inputEl = document.querySelector("#autocomplete-input");

inputEl.addEventListener("input", onInputChange);
getTitleData();

const trie = new Trie();

function readTextFile(file, callback) {
    var rawFile = new XMLHttpRequest();
    rawFile.overrideMimeType("application/json");
    rawFile.open("GET", file, true);
    rawFile.onreadystatechange = function () {
        if (rawFile.readyState === 4 && rawFile.status == "200") {
            callback(rawFile.responseText);
        }
    }
    rawFile.send(null);
}

async function getTitleData() {
    readTextFile("/static/id.json", function (text) {
        const data = JSON.parse(text);
        data.forEach((doc) => {
            if (doc.title.length > 2) {
                trie.insert(doc.title.toLowerCase());
            }
        });
    });
}

function onInputChange() {
    removeAutocompleteDropdown();

    const value = inputEl.value.toLowerCase();
    if (value.length === 0) return;

    // const filteredNames = trie.find(value);
    const filteredNames = trie.find(value).sort(function (a, b) {
        return a.length - b.length || a.localeCompare(b);
    }).slice(0, 10);

    createAutocompleteDropdown(filteredNames);
}

function createAutocompleteDropdown(list) {
    const listEl = document.createElement("ul");
    listEl.className = "autocomplete-list";
    listEl.id = "autocomplete-list";

    list.forEach((title) => {
        const listItem = document.createElement("li");

        const coutryButton = document.createElement("button");
        coutryButton.innerHTML = title;
        coutryButton.addEventListener("click", onTitleButtonClick);
        listItem.appendChild(coutryButton);

        listEl.appendChild(listItem);
    });

    document.querySelector("#autocomplete-wrapper").appendChild(listEl);
}

function removeAutocompleteDropdown() {
    const listEl = document.querySelector("#autocomplete-list");
    if (listEl) listEl.remove();
}

function onTitleButtonClick(event) {
    event.preventDefault();

    const buttonEl = event.target;
    inputEl.value = buttonEl.innerHTML;

    removeAutocompleteDropdown();
}