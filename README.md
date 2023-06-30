# Features

1. Implement Set, implement underlying RedBlack::Tree
2. Implement all functions of std::set from Standard Library
3. Code and comments are made simple and understandable

## Differences To std::set
| RedBlack::Tree                                  | std::set                                     |
|-------------------------------------------------|----------------------------------------------|
| Does not need const_iterator anywhere           | Distinct iterator + const_iterator   types   |
| iterator + reverse_iterator are<br/> same type (interconvertible) |Distinct iterator + reverse_iterator types  |
| Range insert or erase returns size_t            | Range insert or erase returns void           |                                 
| Initialize data directly thru *                 | Initialize data thru <> Alloc wrapper        |                   
| count(key) returns bool                         | count(val) returns size_t                    |

## Functions

### Iterators
```
iterator begin ()
iterator end   ()
iterator rbegin()
iterator rend  ()
```

### Modifiers
```
size_t insert(Iter it, Iter end)       : Count of keys inserted
size_t insert(initializer_list<T> keys): Count of keys inserted
pair<iterator, bool> insert(T& key)
pair<iterator, bool> emplace(Args&&...args)
```
```
size_t erase(Iter it, Iter end)       : Count of keys erased
size_t erase(initializer_list<T> keys): Count of keys erased
pair<iterator, bool> erase(T& key)
pair<iterator, bool> erase(Iter it)
```
```
void swap(Set& a, Set& b)
void clear()
```
### Operations
```
bool     count(T& key): If key is in Set, true
iterator find (T& key)
```

```
iterator lower_bound(T& key)
iterator lower_bound(T& key)
pair<iterator, iterator> equal_range(T& key)
```
### Observers
```
size_t size ()
bool   empty()
```
```
key_compare   key_comp  ()
value_compare value_comp()
```


### std::set Reference:
[https://cplusplus.com/reference/set/set/](https://cplusplus.com/reference/set/set/)
                          
