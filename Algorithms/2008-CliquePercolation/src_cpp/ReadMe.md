## Issues

- in [TableWithStatus.H](lcelib/containers/indices/TableWithStatus.H)

> no match for ‘operator[]’ (operand types are ‘const ArrayBase<bool>’ and ‘const size_t {aka const long unsigned int}’)
     bool isUsed(const size_t i) const {return status[i];}  

- in [](lcelib/containers/tables/StubBase.H)

> lvalue required as unary ‘&’ operand
       memcpy(&(target.directRefTo(loc)), &value, &sizeof(ArrayType::ValueType));