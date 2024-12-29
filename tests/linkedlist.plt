namespace ds
{
  class ListNode
  {
    var val = nil
    var next = nil
    fn __construct__(var val)
    {
      self.val = val
    }
  }
  class LinkedList
  {
    private var head = nil
    private var size = 0
    fn push_back(var node)
    {
      if(self.size==0)
      {
        self.head = node
      }
      else
      {
        var ptr = self.head
        while(ptr.next != nil)
        {
          ptr = ptr.next
        }
        ptr.next = node
      }
      self.size+=1
    }
    fn pop_back()
    {
      var ptr = self.head
      var second = self.head
      while(ptr.next != nil)
      {
        second = ptr
        ptr = ptr.next
      }
      second.next = nil
    }
    fn display()
    {
      var ptr = self.head
      while(ptr.next != nil)
      {
        print(ptr.val,"->")
        ptr = ptr.next
      }
      println(ptr.val)
    }
    fn at_pos(var i)
    {
      if(i>=self.size)
      {
        println("at_pos(): Not enough elements!")
        exit()
      }
      var k = 0
      var ptr = self.head
      while(k<i)
      {
        ptr = ptr.next
        k+=1
      }
      return ptr.val
    }

  }#end class
} # end namespace
var list = ds::LinkedList()
for(var i=10 to 50 step 10)
{
  list.push_back(ds::ListNode(i))
}
list.display()
println("list[3] = ",list.at_pos(3))
