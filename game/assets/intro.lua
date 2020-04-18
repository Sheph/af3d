local objs = scene:getObjects();
for _, obj in pairs(objs) do
    print("obj "..obj.name..", "..obj.cookie);
end

--UIAddTimeout0(1, function(cookie, dt)
--    print(dt);
--end);
