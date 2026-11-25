import { CheckCircle, X, Minus, Square, Info } from "lucide-react";

export const icons = {
  check: CheckCircle,
  Close: X,
  Minus: Minus,
  Square: Square,
  info: Info
} as const;

export type IconName = keyof typeof icons;