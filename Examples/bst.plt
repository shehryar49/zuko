# Binary Search Tree implementation in Plutonium
# Written by Shahryar Ahmad
# 21 Oct 2022
# The code is completely free to use/modify and comes without any guarantee or
# warrantee
namespace bst
{
  class BNode
  {
    var val = nil
    var left = nil
    var right = nil
    function __construct__(var val)
    {
      self.val = val
    }
  }
  class BST
  {
    private var root = nil
    private function insertRecursive(var r,var val)
    {
      if(val < r.val)
      {
        if(r.left == nil)
        {
          r.left = BNode(val)
          return 0
        }
        self.insertRecursive(r.left,val)
      }
      else if(val > r.val)
      {
        if(r.right == nil)
        {
          r.right = BNode(val)
          return 0
        }
        self.insertRecursive(r.right,val)
      }
    }
    private function InOrder(var r)
    {
      if(r==nil)
        return 0
      self.InOrder(r.left)
      println(r.val)
      self.InOrder(r.right)
    }
    private function searchRecursive(var r,var val)
    {
      if(r==nil)
        return nil
      if(r.val == val)
        return r
      else if(val < r.val)
        return self.searchRecursive(r.left,val)
      else if(val > r.val)
        return self.searchRecursive(r.right,val)
    }
    private function Tabular(var r,var spaces)
    {
      for(var i=1 to spaces step 1)
        print(" ")
      if(r==nil)
      {
        println("|nil")
        return 0
      }
      println("|",r.val)
      self.Tabular(r.left,spaces+2)
      self.Tabular(r.right,spaces+2) 
    }
    function insert(var val)
    {
      if(self.root==nil)
      {
        self.root = BNode(val)
      }
      else
        self.insertRecursive(self.root,val)
    }
    function printInOrder()
    {
      self.InOrder(self.root)   
    }
    function min()
    {
      if(self.root == nil)
        return nil
      var p = self.root
      while(p.left != nil)
      {
        p = p.left
      }
      return p.val
    }
    function max()
    {
      if(self.root == nil)
        return nil
      var p = self.root
      while(p.right != nil)
      {
        p = p.right
      }
      return p.val
    }
    function search(var val)
    {
      var r = self.searchRecursive(self.root,val)
      return r!=nil
    }
    function printTabular()
    {
      self.Tabular(self.root,0)
    }
    function delete(var val)
    {
      if(self.root == nil)
        return 0
      var p = self.root
      var parent = nil
      var found = false
      while(p!=nil)
      {
        if(val==p.val)
        {
          found = true
          break
        }
        else if(val < p.val)
        {
          parent = p
          p = p.left
        }
        else if(val > p.val)
        {
          parent = p
          p = p.right
        }
      }
      if(!found)
        return 0
      if((p.left == nil) and (p.right == nil))
      {
        if(parent == nil)
        {
          self.root = nil # only one node was present 
          return 0
        }
        if(parent.left!=nil and (parent.left is p))
          parent.left = nil
        else if(parent.right!=nil and (parent.right is p))
          parent.right = nil
      }
      else if(p.left!=nil and p.right!=nil)
      {
        #find max element from left subtree
        var k = p.left
        var l = nil
        while(k.right!=nil)
        {
          l = k
          k = k.right
        }
        if(l!=nil)
          l.right = k.left
        else
          p.left = k.left
        p.val = k.val

      }
      else if(p.left == nil)
      {
        if(parent.left is p)
          parent.left = p.right
        else if(parent.right is p)
          parent.right = p.right
      }
      else if(p.right == nil)
      {
        if(parent.left is p)
          parent.left = p.left
        else if(parent.right is p)
          parent.right = p.left
      }
    }
  }
}