local v1 = Vec3(1, 2, 3);
local v2 = Vec3(3, 4, 5);

print((v1 + v2) * 2);
print(3 * v2);

local v5 = Vec3(3, 4, 5);
local v6 = Vec3(3, 4, 5);
print(v5 == v6);
print(-v5);

print(Vec3.zeroNormalized(Vec3(2,3,4)));
print(Vec3.zero);
print(Vec3.forward * 5.0);
assert(Vec3.forward ~= Vec3.back);
print(Vec3.forward ~= Vec3(0, 0, -1));
print(tostring(Vec3.forward:dot(Vec3.right)).." = 0");

local objs = scene:getObjects();
for _, obj in pairs(objs) do
    print("obj "..obj.name..", "..obj.cookie);
end
