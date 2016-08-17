
namespace mjson
{
    JSON_INLINE Reference::Reference()
    : counter_(0)
    {}
    
    JSON_INLINE Reference::~Reference()
    {}
    
    JSON_INLINE void Reference::retain()
    {
        ++counter_;
    }
    
    JSON_INLINE void Reference::release()
    {
        if(--counter_ <= 0)
        {
            delete this;
        }
    }
    
    JSON_INLINE long Reference::getRefCount() const
    {
        return counter_;
    }
}
