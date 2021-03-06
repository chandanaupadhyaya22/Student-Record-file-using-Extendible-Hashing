#include <string>
#include <cassert>
#include <iostream>
#include <cstdio>
#include <cstdlib>

int hashCode(std::string const & key)
/*
 * calculates the hash of key using polynomial expansion,
 * with x = 37
 */
{
  const long x = 41L;
  int n = key.length();
  long value = 0L;
  while( n-- > 0 ) {
    value *= x;   value += key[n];
  }
  return static_cast<int>(value);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
struct Student
{
  std::string entryNo;
  std::string name;
  std::string sem;
  std::string branch;
};

std::ostream& operator << (std::ostream& out, Student& s) {
  out << s.entryNo << " : " << s.name<<":"<<s.sem<<":"<<s.branch;
  return out;
}
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------


struct Node
/* sentinal nodes of linked list */
{
  Node *next;
};

struct Pair {
  int first;
  Student *second;

  ~Pair() {
    if(second != NULL)
      delete second;
  }
};

struct InternalNode : public Node
/* internal nodes of linked list */
{
  Pair *element;

  ~InternalNode() {
    delete element;
  }
};

class LinkedList
/*
 * buckets in hash table
 */
{
public:
  LinkedList() {
    head = new Node();
    head->next = NULL;
  }
  ~LinkedList() {
    del(head);
  }
  void insert(Student & elem, int hashkey) {
    // create hash, data pair
    Pair *data = new Pair();
    data->first = hashkey;
    data->second = &elem;
    // create node in list
    InternalNode *temp = new InternalNode();
    temp->next = head->next;
    temp->element = data;
    head->next = temp;
  }
  Student* search(int hashkey) {
    Node *current = head->next;
    while(current != NULL) {
      if(static_cast<InternalNode*>(current)->element->first == hashkey)
        return static_cast<InternalNode*>(current)->element->second;
      current = current->next;
    }
    return NULL;
  }
  void remove(int hashkey) {
    Node *current = head->next;
    Node *parent = head;
    while(current != NULL) {
      if(static_cast<InternalNode*>(current)->element->first == hashkey) {
        // fix links in list then remove and return
        parent->next = current->next;
        current->next = NULL;
        del(current);
        return;
      }
      parent = current;
      current = current->next;
    }
  }
  void removeAll() {
    del(head->next);
    head->next = NULL;
  }
  void print(){
    InternalNode *current = static_cast<InternalNode*>(head->next);
    int i = 1;
    while(current != NULL) {
      std::cout << i << " : hashkey(0x" << std::hex << static_cast<unsigned int>( current->element->first ) << ") <" << *(current->element->second) << ">" << std::endl;
      current = static_cast<InternalNode*>(current->next);
      ++i;
    }
  }
  Node *getList() {
    return head->next;
  }
private:
  void del(Node *node) {
    if(node == NULL)
      return;

    del(node->next);
    delete node;
  }
  Node *head;
};

class Bucket
/*
 * uses LinkedList for storage
 * bucket overflow and underflow is left for xtendedHashing functions
 * they should check for underflow and overflow in bucket, otherwise
 * program will terminate
 */
{
public:
  Bucket(int c, int s = 0) : sig_bits(s), cap(c), sz(0), code(0), root(false) {
    data = new LinkedList();
  }
  ~Bucket() {
    delete data;
  }
  int capacity() {
    return cap;
  }
  int size() {
    return sz;
  }
  void insert(Student& elem, int hashkey) {
    data->insert(elem, hashkey);
    ++sz;
    if(root)
      return;
    assert( ( size() <= capacity() ) );
  }
  Student* search(int hashkey) {
    return data->search(hashkey);
  }
  void remove(int hashkey) {
    if(search(hashkey) != NULL)
      --sz;
    data->remove(hashkey);
    assert( (size() >= 0) );
  }
  void emptyBucket() {
    data->removeAll();
    sig_bits = 0;
    sz = 0;
  }
  bool isEmpty() {
    return size() == 0;
  }
  void print() {
    std::cout << "----- <BUCKET (";
    printcode();
    std::cout << ") > -----" << std::endl;
    std::cout << "SIG_BITS : " << sig_bits << std::endl;
    data->print();
    std::cout << std::endl;
  }
  Node* getList() {
    return data->getList();
  }
  int getSIG_BITS() {
    return sig_bits;
  }
  void setSIG_BITS(int s) {
    sig_bits = s;
    assert( (sig_bits >= 0) );
  }
  void setCODE(int c) {
    code = c;
  }
  void setROOT(bool state) {
    root = state;
  }
private:
  void printcode() {
    int length = sig_bits;
    int temp = code;
    while(length-- > 0) {
      std::cout << temp%2;
      temp >>= 1;
    }
  }
  LinkedList *data;
  int sig_bits;
  int cap;
  int sz;
  int code;
  bool root;
};

struct TrieNode
{
  Bucket *page;
  TrieNode *next[2];
  TrieNode *parent;

  ~TrieNode() {
    if(page != NULL)
      delete page;
  }
};

class HashTable
/*
 * at last we are at the implementation of Extendible-hashTable ^v^
 * uses tries index for bucket lookup
 * 
 */
{
public:
  HashTable(int buk = DFL_BUCKET_SIZE) : bucket_size(buk) {
    sig_bits = 0;
    //root = false;
    index = new TrieNode();
    index->page = new Bucket(bucket_size, sig_bits);
    index->next[0] = NULL;
    index->next[1] = NULL;
    index->parent = NULL;
    no_of_buckets = 1;
  }
  ~HashTable() {
    del(index);
  }
  Student* search(std::string number) {
    int code;
    int hashkey = hashCode(number);
    TrieNode *node = findNode(hashkey, &code);
    return node->page->search(hashkey);
  }
  void insert(Student stu) {
    Student & st = *new Student();
    st.name = stu.name;
    st.entryNo = stu.entryNo;
    st.sem=stu.sem;
    st.branch=stu.branch;
    int hashkey = hashCode(st.entryNo);
    int code;
    TrieNode *node = findNode(hashkey, &code);
    Bucket *page = node->page;
    //if(root || page->size()+1 <= page->capacity())
    if(page->size()+1 <= page->capacity())
    {
      // insert in bucket
      // page->setROOT(root);
      page->insert(st, hashkey);
      // page->setROOT(false);
      return;
    } else {    // bucket is full, make a new bucket
      // step 7
      Bucket *newPage = new Bucket(bucket_size);
      no_of_buckets++;
      // step 8
      LinkedList *Q = new LinkedList();
      InternalNode *current = static_cast<InternalNode*>( page->getList() );
      while(current != NULL) {
        Q->insert(*(current->element->second), current->element->first);
        current = static_cast<InternalNode*>(current->next);
      }
      Q->insert(st, hashkey);
      // step 9
      int oldD = page->getSIG_BITS();
      int MASK = 1 << oldD;
      newPage->setSIG_BITS(oldD + 1);
      // step 10
      page->emptyBucket();
      page->setSIG_BITS(oldD + 1);
      if(oldD+1 > sig_bits){
        ++sig_bits;
      }
        node->page = NULL;
        TrieNode *left = new TrieNode();
        TrieNode *right = new TrieNode();
        node->next[0] = left;
        node->next[1] = right;
        left->page = page;
        right->page = newPage;
        left->next[0] = left->next[1] = NULL; left->parent = node;
        right->next[0] = right->next[1] = NULL; right->parent = node;
        left->page->setCODE(code);
        right->page->setCODE(code | MASK);

      current = static_cast<InternalNode*>( Q->getList() );
      // root = true;
      while(current != NULL) {
        insert( *(current->element->second) );
        current = static_cast<InternalNode*>( current->next );
      }
      // root = false;
    }
  }
  void remove(std::string number){
    int hashkey = hashCode(number);
    int code;
    TrieNode *node = findNode(hashkey, &code);
    Bucket *page = node->page;
    if(page->search(hashkey) == NULL)
      return;
    page->remove(hashkey);
    if(page->size() == 0 && node->parent != NULL)
    {
      TrieNode *parent = node->parent;
      TrieNode *sibling = (parent->next[0] == node)?
            (parent->next[1]) : (parent->next[0]);
      if(sibling->next[0] != NULL || sibling->next[1] != NULL)
        return;

      no_of_buckets--;
      parent->page = sibling->page;
      sibling->page = NULL;
      parent->next[0] = NULL;
      parent->next[1] = NULL;
      del(node);
      del(sibling);
      parent->page->setSIG_BITS(parent->page->getSIG_BITS() - 1);
      int MASK = 1 << parent->page->getSIG_BITS();
      MASK -= 1;
      code &= MASK;
      parent->page->setCODE(code);
      sig_bits = max_bits(index);
    }
  }
  void print() {
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "bucket_capacity : " << bucket_size << "\t|||\t";
    std::cout << "number ( buckets ) : " << no_of_buckets << "\t|||\t";
    std::cout << "max significant_bits : " << sig_bits << std::endl;
    int total = printNode(index);
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "elements in the hashtable : " << total << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
  }
private:
  TrieNode* findNode(int hashkey, int *const code)
  // using lower sig_bits of hashCode to calculate key
  {
    int d = 1;  // mask for calculating key from k
    d <<= sig_bits;
    d -= 1;   // all ones
    int key = hashkey & d;
    *code = 0;
    int length = 0;
    TrieNode *current = index;
    while(current != NULL) {
      if(current->page != NULL)
        return current;
      if(current->next[key & 1] != NULL){
        current = current->next[key & 1];
        int MASK = key&1; // current decision
        if(length != 0)
          MASK <<= length;
        *code |= MASK;
        key >>= 1;
        ++length;
      }
    }
    assert( 0 );  // should not come here.
    return NULL;
  }
  void del(TrieNode *node) {
    if(node == NULL)
      return;
    TrieNode *left, *right;
    left = node->next[0];
    right = node->next[1];
    del(left);
    del(right);
    delete node;
  }
  int printNode(TrieNode *current) {
    if(current == NULL)
      return 0;
    if(current->page != NULL) {
      current->page->print();
      return current->page->size();
    }
    int number = printNode(current->next[0]);
    number += printNode(current->next[1]);
    return number;
  }
  int max_bits(TrieNode *node) {
    if(node->page != NULL)
      return node->page->getSIG_BITS();
    int left = max_bits(node->next[0]);
    int right = max_bits(node->next[1]);
    int max = (left > right) ? left : right;
    return max;
  }
  const static int DFL_BUCKET_SIZE = 4;   // default bucket size
  TrieNode *index;
  int sig_bits;
  int bucket_size;
  int no_of_buckets;
  //bool root;
};

int printMenu() {
  //std::system("clear");
  std::cout << "************************************************************" << std::endl;
  std::cout << "Student Record File Using Extendible Hashing Implementation." << std::endl;
  std::cout << "************************************************************" << std::endl;
  std::cout << "\t\t 1. Insert a Record" << std::endl;
  std::cout << "\t\t 2. Search a Record" << std::endl;
  std::cout << "\t\t 3. Delete a Record" << std::endl;
  std::cout << "\t\t 4. Show All Records"<< std::endl;
  std::cout << "\t\t 5. Delete All Records" << std::endl;
  std::cout << "\t\t 6. Quit" << std::endl;
  int choice;
  std::cout << "\nEnter Your Choice : ";
  std::cin >> choice;
  std::cout.flush();
  return choice;
}

void credits() {
  //std::system("clear");
  std::cout << "\n\n\n";
  std::cout << "\t\t\t  ----------------------------------------------" << std::endl;
  std::cout << "\t\t\t /\\--------------------------------------------/\\" << std::endl;
  std::cout << "\t\t\t|| Thanks for evaluating our Extendible Hashing ||" << std::endl;
  std::cout << "\t\t\t|| \t\tImplementation.\t\t\t||" << std::endl;
  std::cout << "\t\t\t \\/--------------------------------------------\\/" << std::endl;
  std::cout << "\t\t\t  ----------------------------------------------" << std::endl;
  std::cout << "\n\n\t\tImplementors:-\n";
  std::cout << "\t\t\t2006ECS20\tChandana G Upadhyaya\t(Coder)" << std::endl;
  std::cout << "\t\t\t2006ECS57\t6th semester ISE\t\t(RVCE)" << std::endl;
  std::cout << "\n\nPress enter to quit...";
  std::getchar();
}

int main()
{
  std::cout << "Enter the bucket-size : ";
  int n;
  char ch;
  std::cin >> n;
  HashTable *table = new HashTable(n);
  while(true) {
    int choice = printMenu();
    ch = std::getchar();  // eat newline
    switch(choice) {
      case 1:   // Insert a record
      {
        Student st;
        std::cout << "\nEnter Student Name : ";
        std::getline(std::cin, st.name);
        std::cout << "Enter your USN Number : ";
        std::getline(std::cin, st.entryNo);
	std::cout<<"Enter your sem  : ";
	std::getline(std::cin,st.sem);
        std::cout<<"Enter your branch :";
	std::getline(std::cin,st.branch);
        table->insert(st);
        break;
      }
      case 2:   // search a Record
      {
        std::string entNum;
        std::cout << "\nEnter the USN Number: ";
        std::getline(std::cin, entNum);
        Student *st = table->search(entNum);
	if (st->entryNo==entNum)
	{
        	std::cout << *st;
       	 	std::cout << std::endl << "Press enter to continue ...";
        	ch = std::getchar();
        break;
	}
	//else
	//{
		std::cout<<"USN not found!! Press enter to continue...";
		ch=std::getchar();
		break;
	//}
      }
      case 3:   // delete a Record
      {
        std::string entNum;
        std::cout << "\nEnter the USN Number: ";
        std::getline(std::cin, entNum);
        table->remove(entNum);
        break;
      }
      case 4:   // show All Records
        //std::system("clear");
        table->print();
        std::cout << std::endl << "Press enter to continue ...";
        ch = std::getchar();
        break;
      case 5:   // clear All Data
        delete table;
        table = new HashTable(n);
        break;
      case 6:   // Exit
        goto DONE;
    }
  }
  DONE:
  delete table;
 // credits();
  return 0;
}
