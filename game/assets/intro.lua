local box1 = scene:getObjects("box1")[1];

local center = box1:transform().origin + (Vec3.forward * 5.0);

addTimeout0(function(cookie, dt)
    local xfRel = box1:transform();
    xfRel.origin = xfRel.origin - center;

    local xf = Transform(Quaternion(Vec3(0, 1, 0), math.rad(180) * dt), Vec3.zero) * xfRel;
    xf.origin = xf.origin + center;

    box1:setTransform(xf);
end);

local total = 0;
setSensorListener("sensor1", function(other)
    total = total + 1;
    print("entered "..other.name.."("..total..")");
end, function (other)
    total = total - 1;
    print("exited "..other.name.."("..total..")");
end);
