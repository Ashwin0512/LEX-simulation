#include <bits/stdc++.h>
using namespace std;

pair<string, vector<string>> parseFile(const char *path) {
    pair<string, vector<string>> data;
    string s;
    ifstream file(path);

    if (file.is_open()) {
        if (getline(file, data.first)) {
            while (getline(file, s)) {
                data.second.push_back(s);
            }
        } else {
            cerr << "Error: File is empty." << endl;
            exit(0);
        }
    } else {
        cerr << "Error: Unable to open file.\n" << endl;
        exit(0);
    }
    return data;
}

string addConcatenation(string s) {    
    for(int i=0; i<s.length(); i++) {
        if(s[i]==')') {
            if(i!=s.length()-1 && s[i+1]=='(') {
                s.insert(i+1, ".");
            }
        } 
    }
    return s;
}

int precedence(char c) {   
    if (c=='*' || c=='+' || c=='?')
        return 3;
    else if(c=='.')
        return 2;
    else if(c=='|')
        return 1;
    else 
        return -1;
}

string infixToPostfix(string reg) { 
    reg = addConcatenation(reg);
    stack<char> s;
    string res;
    for(int i=0; i<reg.size(); i++) {
        char c = reg[i];
        if(c=='a' || c=='b') {
            res += c;
        } else if (c=='(') {
            s.push('(');
        } else if(c==')') {
            while(s.top() != '(') {
                res += s.top();
                s.pop();
            }
            s.pop();
        } else {
            while(!s.empty() && precedence(s.top()) >= precedence(c)) {
                res += s.top();
                s.pop();
            }
            s.push(c);
        }
    }

    while(!s.empty()) {
        res += s.top();
        s.pop();
    }

    return res;
}

struct NfaEpsilon {
    int startState;
    vector<pair<int,array<unordered_set<int>,3>>> transitions;
    int finalState;
};

NfaEpsilon createNode(char c, int &stateNum) {  
    NfaEpsilon nfa;
    nfa.startState = stateNum;
    stateNum++;
    nfa.finalState = stateNum;
    stateNum++;

    array<unordered_set<int>,3> temp;

    nfa.transitions.push_back(make_pair(nfa.startState, temp));
    nfa.transitions.push_back(make_pair(nfa.finalState, temp));

    nfa.transitions[0].second[c-'a'].insert(nfa.finalState);
    return nfa;
}

NfaEpsilon concatenate(NfaEpsilon nfa1, NfaEpsilon nfa2, int &stateNum) {
    NfaEpsilon nfa;
    nfa.startState = nfa1.startState;
    nfa.finalState = nfa2.finalState;
    
    int t1 = nfa1.transitions.size();
    int t2 = nfa2.transitions.size();

    int fs = nfa1.transitions[0].first;
    int offset = nfa1.finalState - fs;
    nfa1.transitions[offset].second[2].insert(nfa2.startState);

    for(int i=0; i<t1; i++)
        nfa.transitions.push_back(nfa1.transitions[i]);
    for(int i=0; i<t2; i++)
        nfa.transitions.push_back(nfa2.transitions[i]);
    
    return nfa;
}

NfaEpsilon makeUnion(NfaEpsilon nfa1, NfaEpsilon nfa2, int &stateNum) {
    NfaEpsilon nfa;
    nfa.startState = stateNum;
    stateNum++;
    nfa.finalState = stateNum;
    stateNum++;

    int t1 = nfa1.transitions.size();
    int t2 = nfa2.transitions.size();

    for(int i=0; i<t1; i++)
        nfa.transitions.push_back(nfa1.transitions[i]);
    for(int i=0; i<t2; i++)
        nfa.transitions.push_back(nfa2.transitions[i]);
    
    pair<int,array<unordered_set<int>,3>> p;
    p.first = nfa.startState;
    p.second[2].insert(nfa1.startState);
    p.second[2].insert(nfa2.startState);
    nfa.transitions.push_back(p);

    pair<int,array<unordered_set<int>,3>> p2;
    p2.first = nfa.finalState;
    nfa.transitions.push_back(p2);

    int fs = nfa1.transitions[0].first;
    int offset = nfa1.finalState - fs;
    nfa.transitions[offset].second[2].insert(nfa.finalState);

    int offset2 = t1+nfa2.finalState-nfa2.transitions[0].first;
    nfa.transitions[offset2].second[2].insert(nfa.finalState);

    return nfa;
}

NfaEpsilon kleeneClosure(NfaEpsilon nfa, int &stateNum) {
    int prev = nfa.startState;
    int prevFinal = nfa.finalState;
    nfa.startState = stateNum;
    stateNum++;
    nfa.finalState = stateNum;
    stateNum++;

    pair<int,array<unordered_set<int>,3>> p;
    p.first = nfa.startState;
    p.second[2].insert(prev);
    p.second[2].insert(nfa.finalState);
    nfa.transitions.push_back(p);

    pair<int,array<unordered_set<int>,3>> p2;
    p2.first = nfa.finalState;
    p2.second[2].insert(nfa.startState);
    nfa.transitions.push_back(p2);

    for(int i=0; i<nfa.transitions.size(); i++) {
        pair<int,array<unordered_set<int>,3>> a = nfa.transitions[i];
        if(a.first == prevFinal) {
            nfa.transitions[i].second[2].insert(nfa.finalState);
        } 
    }

    return nfa;
}

NfaEpsilon positiveClosure(NfaEpsilon nfa, int &stateNum) {
    int prevFinal = nfa.finalState;
    nfa.finalState = stateNum;
    stateNum++;

    pair<int,array<unordered_set<int>,3>> p;
    p.first = nfa.finalState;
    nfa.transitions.push_back(p);

    for(int i=0; i<nfa.transitions.size(); i++) {
        pair<int,array<unordered_set<int>,3>> a = nfa.transitions[i];
        if(a.first == prevFinal) {
            nfa.transitions[i].second[2].insert(nfa.startState);
            nfa.transitions[i].second[2].insert(nfa.finalState);
        }
    }

    return nfa;
}

NfaEpsilon atMostOne(NfaEpsilon nfa, int &stateNum) {
    int prevFinal = nfa.finalState;
    nfa.finalState = stateNum;
    stateNum++;
    
    pair<int,array<unordered_set<int>,3>> p;
    p.first = nfa.finalState;
    nfa.transitions.push_back(p);

    for(int i=0; i<nfa.transitions.size(); i++) {
        pair<int,array<unordered_set<int>,3>> a = nfa.transitions[i];
        if(a.first == prevFinal || a.first == nfa.startState) {
            nfa.transitions[i].second[2].insert(nfa.finalState);
        }
    }
    return nfa;
}

NfaEpsilon constructNfaEpsilon(string reg) {
    reg = addConcatenation(reg);
    reg = infixToPostfix(reg);

    stack<char> st;
    stack<NfaEpsilon> nfaStack;
    int stateNum = 0;

    for(char c:reg) {
        if(c=='a' || c=='b') {
            nfaStack.push(createNode(c, stateNum));
        } else if(c=='.') {
            NfaEpsilon nfa2 = nfaStack.top();
            nfaStack.pop();
            NfaEpsilon nfa1 = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(concatenate(nfa1,nfa2,stateNum));
        } else if(c=='|')  {
            NfaEpsilon nfa2 = nfaStack.top();
            nfaStack.pop();
            NfaEpsilon nfa1 = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(makeUnion(nfa1,nfa2,stateNum));
        } else if(c=='*') {
            NfaEpsilon nfa = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(kleeneClosure(nfa,stateNum));
        }  else if(c=='+') {
            NfaEpsilon nfa = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(positiveClosure(nfa,stateNum));
        } else if(c=='?') {
            NfaEpsilon nfa = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(atMostOne(nfa,stateNum));
        }
    }
    return nfaStack.top();
}

struct Nfa {
    int startState;
    vector<pair<int,array<unordered_set<int>,2>>> transitions;
    vector<int> finalStates;
};

unordered_set<int> epsilonClosure(const NfaEpsilon& nfa, int state) {
    unordered_set<int> closure;
    vector<bool> visited(nfa.transitions.size(), false);

    function<void(int)> dfs = [&](int s) {
        closure.insert(s);
        visited[s] = true;
        for (int nextState : nfa.transitions[s].second[2]) {
            if (!visited[nextState]) {
                dfs(nextState);
            }
        }
    };

    dfs(state);
    return closure;
}

Nfa convertToNfa(NfaEpsilon nfaEpsilon) {
    Nfa nfa;
    nfa.startState = nfaEpsilon.startState;
    nfa.finalStates.push_back(nfaEpsilon.finalState);

    unordered_set<int> epClStart = epsilonClosure(nfaEpsilon, nfaEpsilon.startState);
    if(epClStart.find(nfaEpsilon.finalState) != epClStart.end())
        nfa.finalStates.push_back(nfaEpsilon.startState);

    for(int i=0; i<nfaEpsilon.transitions.size(); i++) {
        pair<int,array<unordered_set<int>,2>> pr;
        pr.first = i;
    
        unordered_set<int> stCl = epsilonClosure(nfaEpsilon,i);

        unordered_set<int> epStCl;
        for(auto num:stCl) {
            unordered_set<int> p = nfaEpsilon.transitions[num].second[0];
            epStCl.insert(p.begin(), p.end());
        }

        for(auto num:epStCl) {
            unordered_set<int> ins = epsilonClosure(nfaEpsilon, num);
            pr.second[0].insert(ins.begin(), ins.end());
        }

        unordered_set<int> epStCl2;
        for(auto num:stCl) {
            unordered_set<int> p = nfaEpsilon.transitions[num].second[1];
            epStCl2.insert(p.begin(), p.end());
        }

        for(auto num:epStCl2) {
            unordered_set<int> ins = epsilonClosure(nfaEpsilon, num);
            pr.second[1].insert(ins.begin(), ins.end());
        }

        nfa.transitions.push_back(pr);
    }
    return nfa;
}

bool acceptString(Nfa nfa, string s) {
    unordered_set<int> curState;
    curState.insert(nfa.startState);
    bool accept = false;

    for(char c:s) {
        int sym = c-'a';
        unordered_set<int> updatedState;
        for(auto elem:curState) {
            unordered_set<int> st = nfa.transitions[elem].second[sym];
            updatedState.insert(st.begin(), st.end());
        }
        curState = updatedState;
    }

    for(auto i:nfa.finalStates) {
        if (curState.find(i) != curState.end()) {
            accept = true;
            return accept;
        }
    } 
    return accept;
}

vector<pair<string,int>> generateOutput(const string& input, const vector<Nfa>& nfas) {
    int i=0, j=input.size()-1;
    int n = input.size();
    vector<pair<string,int>> ans;

    while(i<n && j<n) { 
        string sub = input.substr(i,j-i+1);
        bool flag = false;
        for(int k=0; k<nfas.size(); k++) {
            if(acceptString(nfas[k],sub)) {
                ans.push_back({sub,k+1});
                flag = true;
                i += sub.size();
                j = input.size()-1;
                break;
            } else {
                if(sub.size()==1 && k==nfas.size()-1) {
                    ans.push_back({sub,0});
                    i++;
                    j = input.size()-1;
                    flag = true;
                    break;
                }
            }
        }
        if(!flag)
            j--;
    }
    return ans;
}

void writeToFile(const vector<pair<string, int>>& ans, const string& filename) {
    ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        cerr << "Error: Unable to open file " << filename << " for writing." << endl;
        return;
    }

    for (const auto& p : ans) {
        outputFile << "<" << p.first << "," << p.second << ">";
    }

    outputFile.close();
    cout << "Content written to file " << filename << " successfully." << endl;
}

int main() {
    const char *path = "input.txt";
    pair<string, vector<string>> data = parseFile(path);
    string inputString = data.first;
    vector<string> regexVec = data.second;

    vector<NfaEpsilon> nfaEpsilons;
    for(auto s:regexVec)
        nfaEpsilons.push_back(constructNfaEpsilon(s));

    vector<Nfa> nfas;
    for(auto n:nfaEpsilons)
        nfas.push_back(convertToNfa(n));

    vector<pair<string,int>> ans = generateOutput(inputString,nfas);
    const string outputPath = "output.txt";

    writeToFile(ans,outputPath);
}
