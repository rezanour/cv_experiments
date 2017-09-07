#pragma once

class NonCopyable
{
protected:
    NonCopyable() = default;
    NonCopyable(NonCopyable const &) = delete;
    NonCopyable &operator=(NonCopyable const &) = delete;
};
