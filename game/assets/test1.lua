local total1 = 0;
setSensorListener("sensor1", function(other)
    total1 = total1 + 1;
    print("entered "..other.name.."("..total1..")");
    other.linearVelocity = -other.linearVelocity;
end, function (other)
    total1 = total1 - 1;
    print("exited "..other.name.."("..total1..")");
end);

local total2 = 0;
setSensorListener("sensor2", function(other)
    total2 = total2 + 1;
    print("entered "..other.name.."("..total2..")");
    other.linearVelocity = -other.linearVelocity;
end, function (other)
    total2 = total2 - 1;
    print("exited "..other.name.."("..total2..")");
end);
