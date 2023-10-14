package trie

type MatchFlags uint32

const (
	MatchFlagsNone            MatchFlags = 0
	MatchFlagsCaseInsensitive MatchFlags = 1 << 0
	MatchFlagsFuzzy           MatchFlags = 1 << 1
)

func (a MatchFlags) Or(b MatchFlags) MatchFlags {
	return a | b
}
func (a MatchFlags) And(b MatchFlags) bool {
	return a&b != 0
}
func (a *MatchFlags) OrEq(b MatchFlags) *MatchFlags {
	*a = a.Or(b)
	return a
}
func (a *MatchFlags) XorEq(b MatchFlags) *MatchFlags {
	*a = a.Xor(b)
	return a
}
func (a MatchFlags) Xor(b MatchFlags) MatchFlags {
	return MatchFlags(uint32(a) ^ uint32(b))
}

func uchar(str []byte) (uint32, int) {
	if len(str) == 0 {
		return 0, 0
	}
	if str[0]&0x80 == 0 {
		return uint32(str[0]), 1
	}
	if str[0]&0x70 == 0x70 {
		return uint32(str[0]&0x07)<<18 | uint32(str[1]&0x3F)<<12 | uint32(str[2]&0x3F)<<6 | uint32(str[3]&0x3F), 4
	}
	if str[0]&0x60 == 0x60 {
		return uint32(str[0]&0x0F)<<12 | uint32(str[1]&0x3F)<<6 | uint32(str[2]&0x3F), 3
	}
	if str[0]&0x40 == 0x40 {
		return uint32(str[0]&0x1F)<<6 | uint32(str[1]&0x3F), 2
	}
	return 0, 0
}

type TrieNode struct {
	children map[uint32]*TrieNode
	infos    []interface{}
}

func NewTrieNode() *TrieNode {
	return &TrieNode{
		children: map[uint32]*TrieNode{},
		infos:    []interface{}{},
	}
}

func (t *TrieNode) TryInsert(word []byte) *TrieNode {
	c, s := uchar(word)
	if s == 0 {
		return t
	}
	if _, ok := t.children[c]; !ok {
		t.children[c] = NewTrieNode()
	}
	return t.children[c].TryInsert(word[s:])
}

func (t *TrieNode) Find(word []byte, flags MatchFlags) []*TrieNode {
	var nodes []*TrieNode
	c, s := uchar(word)
	if s == 0 {
		nodes = append(nodes, t)
		return nodes
	}
	var citer *TrieNode
	if flags&MatchFlagsCaseInsensitive != 0 && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
		if citer = t.children[c^0x20]; citer != nil {
			child_nodes := citer.Find(word[s:], flags)
			nodes = append(nodes, child_nodes...)
		}
	}
	var niter *TrieNode
	if niter = t.children[c]; niter != nil {
		child_nodes := niter.Find(word[s:], flags)
		nodes = append(nodes, child_nodes...)
	}
	if flags&MatchFlagsFuzzy != 0 {
		for _, iter := range t.children {
			if iter == citer || iter == niter {
				continue
			}
			child_nodes := iter.Find(word, flags)
			nodes = append(nodes, child_nodes...)
		}
	}
	return nodes
}

func (t *TrieNode) AllInfo() []interface{} {
	var nodes []interface{}
	if len(t.infos) != 0 {
		nodes = append(nodes, t.infos...)
	}
	for _, child := range t.children {
		child_nodes := child.AllInfo()
		nodes = append(nodes, child_nodes...)
	}
	return nodes
}

func (t *TrieNode) AddInfo(info interface{}) {
	t.infos = append(t.infos, info)
}

func (t *TrieNode) Info() []interface{} {
	return t.infos
}

func (t *TrieNode) Print(callback func(interface{})) {
	for _, child := range t.children {
		for _, info := range child.Info() {
			callback(info)
		}
		child.Print(callback)
	}
}

type Trie struct {
	root *TrieNode
}

func NewTrie() *Trie {
	return &Trie{
		root: NewTrieNode(),
	}
}

func (t *Trie) Insert(word string, info interface{}) {
	t.root.TryInsert([]byte(word)).AddInfo(info)
}

func (t *Trie) FindPrefix(word string, flags MatchFlags) []interface{} {
	var infos []interface{}
	nodes := t.root.Find([]byte(word), flags)
	for _, node := range nodes {
		child_nodes := node.AllInfo()
		infos = append(infos, child_nodes...)
	}
	return infos
}

func (t *Trie) Print(callback func(interface{})) {
	t.root.Print(callback)
}
