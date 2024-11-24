- Some starting values. Misses target points & oscillates a lot.

```
p0.5000
i0.0000
d0.0007
r2000.0000
l5.0000
```

---


- very precise, very 'correct', but weak movements.

```
p0.1000
i0.0010
d0.0003
r2000.0000
l12.0000
```

---


- Very fast, precise, but sometimes wild oscillations

```
p0.9000
i0.0090
d0.0003
r2000.0000
l12.0000
```

---

- Lots of torque, precise, fast
- Sometimes loses precision

```
p0.8000
i0.0090
d0.0003
r2000.0000
l12.0000
```

---

- Very repeatable.
- Still fast, precise, torquey
- Little bit of bounce

```
p0.7000
i0.0090
d0.0003
r2000.0000
l12.0000
```

---

- So far, best results
- Changed to 200ms hops -- not repeatable enough -- misses one side.
- Also, gets hot

```
p0.7000
i0.0090
d0.0002
r2000.0000
l12.0000
```

---


```
p0.7000
i0.0090
d0.0002
r2000.0000
l12.0000
```

---

- Torque is "good". Not great.
- Boppin
- Precise
- Quiet
- Not bad current

```
p0.7000
i0.0120
d0.0002
r2000.0000
l15.0000
```

---

- l is where most torque comes from
- very happy with torque
- worried it's not hitting the points, but it's very precise

```
p0.7000
i0.0240
d0.0002
r2000.0000
l18.0000
f200.00
```
---

- imprecise
- high torque
- kinda bad
- need `r` to be between `1500` - `2500`.

```
p0.7000
i0.0240
d0.0002
r1000.0000
l18.0000
f200.00
```

---

- excellent torque
- meeting target values
- could be faster as approaches target

```
p0.7000
i0.0300
d0.0002
r2000.0000
l15.0000
f1000.00
```

---

- very consistent
- very high torque
- slows down a bit much before reaching target

```
p0.6000
i0.0300
d0.0002
r1500.0000
l12.0000
f50.00
```

---

- good general default

```
p0.7000000
i0.1000000
d0.0002000
r2000.0000000
l12.0000000
f200.00
```
---

- "basically perfect"

```
p0.7000000
i0.0900000
d0.0002000
r2000.0000000
l12.0000000
f200.00
```
---

- Gearbox with heavy resistance
- Working so far. Bit slow.

```
p2.5000000
i9.0000000
d0.0001000
r4000.0000000
l15.0000000
f5000.00
```